# Theoretical Foundations: Advanced Operating System Concepts

---

## 1. Linux Namespaces: Isolation Without Hypervisors

A namespace wraps a global system resource in an abstraction that makes it appear to the processes within the namespace that they have their own isolated instance of the global resource.

| Namespace | Flag | Isolated Resource | Effect |
| :--- | :--- | :--- | :--- |
| **PID** | `CLONE_NEWPID` | Process IDs | The first process in the namespace becomes PID 1 (init); cannot see host processes. |
| **Network** | `CLONE_NEWNET` | Network devices, IP routing tables, port bindings | Dedicated `veth` virtual interface, private loopback, own firewall rules (`iptables`/`nftables`). |
| **Mount** | `CLONE_NEWNS` | Filesystem mount points | Can mount/unmount filesystems without affecting host or other containers. |
| **UTS** | `CLONE_NEWUTS` | Hostname and NIS domain name | Container can set its own hostname (`sethostname()`). |
| **IPC** | `CLONE_NEWIPC` | System V IPC, POSIX message queues | Cannot access host message queues or shared memory objects. |
| **User** | `CLONE_NEWUSER` | UID/GID mappings | A process with UID 0 (root) inside the namespace maps to an unprivileged UID (e.g. 1000) on the host. |
| **Cgroup** | `CLONE_NEWCGROUP` | Root directory of cgroup hierarchy | Prevents container from seeing host cgroup paths. |
| **Time** | `CLONE_NEWTIME` | System monotonic and boot clocks | Can alter perceived uptime/time offsets per container. |

### Syscalls for Namespaces
- `clone(int (*fn)(void*), void* stack, int flags, void* arg)`: Creates a child process in new specified namespaces.
- `unshare(int flags)`: Disassociates parts of the calling process's execution context from its parent.
- `setns(int fd, int nstype)`: Attaches the calling thread to an existing namespace (the foundation of `docker exec`).

---

## 2. Control Groups (cgroups v2)

While namespaces govern **what a process can see**, cgroups govern **how much resource a process can use**.

In cgroups v2 (Unified Hierarchy), all controllers are organized under a single unified hierarchy mounted at `/sys/fs/cgroup`:
- **CPU Controller (`cpu.max`)**: Configured as `<quota> <period>` (e.g., `200000 100000` restricts the cgroup to 2 cores = 200ms per 100ms period).
- **Memory Controller (`memory.max`, `memory.high`)**:
  - `memory.max`: Hard limit. Exceeding triggers synchronous reclamation, and if unsuccessful, the OOM killer terminates a process within the cgroup.
  - `memory.high`: Soft limit. Exceeding throttles the processes and triggers aggressive asynchronous background page reclamation without invoking OOM.
- **I/O Controller (`io.max`)**: Configures read/write IOPS and byte rate caps per block device (e.g., `8:0 rbps=10485760` limits read throughput to 10 MB/s).
- **PIDs Controller (`pids.max`)**: Protects against fork bombs by limiting maximum active threads/processes.

---

## 3. The Linux Out-Of-Memory (OOM) Killer

When available physical RAM and swap are exhausted and kernel page allocation fails:
1. The kernel invokes `out_of_memory()`.
2. Computes the **badness score** for each candidate process:
   $$\text{score} = \frac{\text{RSS} + \text{Swap pages}}{\text{Total RAM}} \times 1000 + \text{oom\_score\_adj}$$
3. The process with the highest score is chosen as the victim.
4. The kernel sends `SIGKILL` to reclaim its entire virtual address space immediately.
- **Tuning via `/proc/[pid]/oom_score_adj`**:
  - Accepts values from `-1000` to `+1000`.
  - `-1000` = Never kill (used by sshd, systemd, Kubernetes Kubelet).
  - `+1000` = Always kill first (used for sacrificial worker processes).

---

## 4. NUMA (Non-Uniform Memory Access)

In modern multi-socket server hardware (and multi-chiplet CPUs like AMD EPYC and Apple Silicon clusters):
- Physical RAM is partitioned into **NUMA nodes**, each directly wired to a specific CPU socket via local memory buses.
- **Local Access**: CPU accessing its own node's RAM achieves maximum bandwidth (~200 GB/s) and lowest latency (~60-80 ns).
- **Remote Access**: CPU accessing memory attached to another socket must traverse inter-socket interconnects (Intel UPI, AMD Infinity Fabric), resulting in **1.5x to 3x higher latency** and reduced bandwidth.
- **Linux Policies (`numactl`)**:
  - `bind`: Strict allocation from specified NUMA nodes only.
  - `interleave`: Round-robins page allocations across all nodes (useful for shared databases).
  - `preferred`: Tries local node first, falls back if full.

---

## 5. Next-Generation Asynchronous I/O: `io_uring`

Traditional Linux I/O models had major bottlenecks:
- **`read()` / `write()`**: Synchronous and blocking; requires 1 syscall per operation.
- **`epoll()`**: Non-blocking readiness notification, but still requires separate `epoll_wait()` followed by individual `read()` / `write()` syscalls (multiple syscalls per I/O event).
- **`io_uring` (Linux 5.1+)**:
  - Shared memory ring buffers between kernel and user space:
    - **Submission Queue (SQ)**: User space writes I/O requests (`io_uring_sqe`) without syscalls.
    - **Completion Queue (CQ)**: Kernel writes completion results (`io_uring_cqe`) without syscalls.
  - In kernel polling mode (`IORING_SETUP_SQPOLL`), a dedicated kernel thread polls the submission queue, enabling **completely syscall-free asynchronous storage and network I/O**!
