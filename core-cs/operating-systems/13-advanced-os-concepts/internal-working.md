# Internal Working: Namespaces, Cgroups, and Memory Topologies

---

## 1. How the Linux Kernel Implements Namespaces (`nsproxy`)

Every process in Linux is represented by `task_struct`. Namespace membership is tracked via a pointer to `struct nsproxy`:

```c
struct task_struct {
    ...
    struct nsproxy *nsproxy;
    ...
};

struct nsproxy {
    atomic_t count;
    struct uts_namespace *uts_ns;
    struct ipc_namespace *ipc_ns;
    struct mnt_namespace *mnt_ns;
    struct pid_namespace *pid_ns_for_children;
    struct net           *net_ns;
    struct cgroup_namespace *cgroup_ns;
    struct time_namespace *time_ns;
};
```

1. **Namespace Sharing**: When a normal process calls `fork()`, the child simply increments `nsproxy->count` and points to the same `nsproxy`.
2. **Namespace Creation**: When `clone()` is called with flags like `CLONE_NEWPID | CLONE_NEWNET`, the kernel allocates a new `nsproxy`, allocates new namespace descriptors for the specified flags, and points the child's `task_struct` to them.
3. **The Special Case of PID Namespaces**:
   - In a new PID namespace, the child becomes PID 1 (`task->pid = 1` inside its namespace, though it has a normal host PID, e.g. 5420, in the parent namespace).
   - If PID 1 inside the container dies or receives an unhandled signal, the kernel terminates **all** other processes within that PID namespace (`SIGKILL` broadcast).

---

## 2. Anatomy of a Minimal Container Runtime

What actually executes when running `docker run` or `podman run`?

```
[ Host Kernel ]
       |
       | 1. clone(CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWUTS, ...)
       v
[ Child Process in New Namespaces ]
       |
       | 2. mount("none", "/", NULL, MS_REC | MS_PRIVATE, NULL)  (Make mounts private)
       | 3. mount(rootfs, rootfs, "bind", MS_BIND | MS_REC, NULL)
       | 4. pivot_root(rootfs, old_root)  (Swap root filesystem to container image)
       | 5. umount2("/old_root", MNT_DETACH)
       | 6. mount("proc", "/proc", "proc", 0, NULL)  (Mount fresh isolated /proc)
       | 7. sethostname("container-host", 14)
       | 8. Apply seccomp BPF filter & drop capabilities (e.g. CAP_SYS_ADMIN)
       v
[ execve("/bin/sh", ...) ]
```

There is **no emulation, no hypervisor, no virtual BIOS, no guest kernel**. A container is literally an ordinary host Linux process wrapped with restricted namespace pointers, cgroup limits, and dropped capabilities.

---

## 3. CPU Cache Hierarchy & Latency Cliffs

Modern CPUs access memory through a multi-tiered hierarchy:

```
+-------------------------------------------------------------+
| CPU Core (ALU / Registers)                                  |
|   Latency: < 0.5 ns                                         |
+-------------------------------------------------------------+
        |
        v
+-------------------------------------------------------------+
| L1 Data Cache (32 KB - 64 KB per core, 8-way associative)   |
|   Latency: ~1 - 1.5 ns (4 - 5 cycles)                       |
+-------------------------------------------------------------+
        |
        v
+-------------------------------------------------------------+
| L2 Cache (512 KB - 4 MB per core, 8-16 way associative)     |
|   Latency: ~3 - 5 ns (14 - 18 cycles)                       |
+-------------------------------------------------------------+
        |
        v
+-------------------------------------------------------------+
| L3 Shared Cache (16 MB - 128 MB shared across cores)        |
|   Latency: ~10 - 20 ns (40 - 70 cycles)                     |
+-------------------------------------------------------------+
        |
        v
+-------------------------------------------------------------+
| Main Memory (DDR4 / DDR5 RAM, Local NUMA Socket)            |
|   Latency: ~60 - 90 ns (200 - 300 cycles)                   |
+-------------------------------------------------------------+
        |
        v
+-------------------------------------------------------------+
| Remote NUMA Socket RAM (Interconnect hop)                   |
|   Latency: ~120 - 250 ns (400 - 800 cycles)                 |
+-------------------------------------------------------------+
```

When traversing memory randomly (pointer chasing / linked lists):
- If the working set fits in L1 cache: memory access takes ~1 ns.
- If the working set spills into main RAM: memory access takes ~80 ns (an **80x latency penalty**!).
- If it spans NUMA sockets: latency jumps by another **2x to 3x**.
