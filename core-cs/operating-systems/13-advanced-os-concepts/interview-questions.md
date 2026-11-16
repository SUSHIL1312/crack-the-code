# Advanced OS Concepts: Top FAANG Interview Questions

### Q1: "A container is just a process." How accurate is this statement, and what kernel primitives make it possible?
**Answer**:
- **Accuracy**: 100% technically accurate. Unlike virtual machines which run a full secondary operating system kernel inside emulated hardware (managed by a hypervisor like KVM), containers run directly on the host kernel.
- **Kernel Primitives**:
  1. **Namespaces (Isolation)**: What the process can *see*. Restricts visibility of process tree (`PID`), network interfaces (`NET`), filesystem mounts (`MNT`), hostnames (`UTS`), IPC queues (`IPC`), and user mappings (`USER`).
  2. **Control Groups / cgroups (Resource Limits)**: What the process can *use*. Enforces quotas on CPU cores/shares (`cpu.max`), physical RAM limits (`memory.max`), disk I/O bandwidth (`io.max`), and thread counts (`pids.max`).
  3. **Linux Capabilities (Privilege Restrictions)**: Divides traditional monolithic root privileges into ~40 fine-grained capabilities (`CAP_NET_BIND_SERVICE`, `CAP_SYS_ADMIN`, `CAP_CHOWN`). Container runtimes drop dangerous capabilities by default.
  4. **Seccomp-BPF (Syscall Filtering)**: Restricts which syscalls the process can invoke. Docker's default seccomp profile disables over 40 syscalls (such as `keyctl`, `reboot`, `sysfs`).
  5. **Chroot / Pivot_root (Filesystem Jailing)**: Changes the process's root directory (`/`) to the unpacked container image layer.

---

### Q2: In Kubernetes, what causes an `OOMKilled` (Exit Code 137) error, and how does the kernel choose which process to terminate?
**Answer**:
1. **Cause of Exit Code 137**:
   - Exit code 137 corresponds to $128 + 9$, meaning the process was terminated by `SIGKILL` (signal 9) sent by the kernel OOM killer.
   - In Kubernetes, a Pod specifies `resources.limits.memory`. The Kubelet translates this into `memory.max` in the Pod's cgroup v2 directory (`/sys/fs/cgroup/kubepods.slice/...`).
   - When the processes in the cgroup consume memory exceeding `memory.max`, the kernel first tries to reclaim inactive file-backed page cache pages. If memory usage remains above `memory.max`, the cgroup OOM killer triggers.
2. **Victim Selection**:
   - The kernel calculates a badness score for every process in the cgroup based on RSS + swap size.
   - Adjusts by `/proc/[pid]/oom_score_adj`. In Kubernetes:
     - **Guaranteed Pods**: `oom_score_adj = -997` (killed last).
     - **Burstable Pods**: `oom_score_adj` between `2` and `999` based on memory requested vs total node RAM.
     - **BestEffort Pods**: `oom_score_adj = 1000` (killed first).
   - The process with the highest score is sent `SIGKILL`.

---

### Q3: What is NUMA and why can it cause unexpected 50% performance drops in high-performance databases (e.g. MySQL, Redis, MongoDB)?
**Answer**:
- **Root Cause**: On multi-socket servers, CPUs have fast local access to their own memory bus, and slower remote access to other sockets' memory across UPI/Infinity Fabric interconnects.
- **The NUMA Problem (Zone Reclaim Flaw)**:
  - If an OS kernel memory allocation policy is strictly node-local (`zone_reclaim_mode = 1`), when NUMA Node 0 runs out of memory, the kernel may aggressively page out disk cache or trigger swap **on Node 0**, even though NUMA Node 1 has 50 GB of free memory available! This causes massive disk I/O stall spikes.
  - Furthermore, if a multi-threaded database thread running on Core 4 (Socket 0) frequently accesses buffer pool pages allocated in Socket 1's RAM, memory latency increases from ~70 ns to ~180 ns, and cross-socket interconnect bandwidth becomes saturated.
- **Remediation**:
  1. Set `vm.zone_reclaim_mode = 0` so the OS allocates from remote nodes rather than reclaiming pages or swapping.
  2. Use `numactl --interleave=all` to evenly distribute buffer pool allocations across all NUMA nodes.
  3. For single-threaded engines (like Redis), bind the process to a single NUMA node and specific core: `numactl --cpunodebind=0 --membind=0 redis-server`.

---

### Q4: Why is `io_uring` considered superior to `epoll` for high-throughput I/O?
**Answer**:
1. **Storage vs Network**:
   - `epoll` only works with pollable file descriptors (sockets, pipes, eventfds). It **does not work** for regular disk files (regular files are always considered ready, so `epoll` blocks on disk I/O).
   - `io_uring` works uniformly across **both disk files and network sockets**.
2. **Syscall Overhead Elimination**:
   - In `epoll`, handling 1,000 requests requires at least 1 `epoll_wait()` syscall, plus 1,000 `read()`/`write()` syscalls (at least 1,001 user-to-kernel context switches).
   - In `io_uring`, the application places 1,000 requests onto the shared memory Submission Queue (SQ) and issues a single `io_uring_enter()` syscall (or **zero syscalls** in `IORING_SETUP_SQPOLL` mode).
3. **Zero-Copy & Fixed Buffers**: `io_uring` supports registered fixed buffers (`io_uring_register_buffers`) where pages are pinned in the kernel once, eliminating repeated virtual memory lookups and page pinning on every I/O operation.
