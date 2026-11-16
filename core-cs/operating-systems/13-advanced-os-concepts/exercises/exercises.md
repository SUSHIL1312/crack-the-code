# Hands-On Exercises: Advanced OS Concepts

---

### Exercise 1: Build a Mini-Container from Scratch (Linux)
**Goal**: Write a standalone C++ program `mini_container` that executes an arbitrary shell command in an isolated container environment.

**Requirements**:
1. Invoke `unshare()` or `clone()` with `CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWNET`.
2. Configure mount propagation to `MS_PRIVATE` using `mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)`.
3. Set up an isolated root filesystem using `pivot_root()` or `chroot()`.
4. Mount a fresh instance of `/proc` inside the container so `ps aux` only shows processes within the container.
5. Set container hostname to `mini-sandbox`.
6. Execute `/bin/sh` or user-specified binary via `execve()`.

---

### Exercise 2: NUMA-Aware Memory Allocator
**Goal**: Implement a C++ NUMA allocator wrapper using `libnuma` (`numa_alloc_onnode`, `numa_free`).

**Requirements**:
1. Query available NUMA nodes on the system using `numa_max_node()`.
2. Allocate a 100 MB buffer on NUMA Node 0.
3. Pin the current thread to a CPU core on NUMA Node 0 using `pthread_setaffinity_np()`, and measure read throughput.
4. Pin the thread to a CPU core on NUMA Node 1 (remote socket), and measure read throughput to the same Node 0 buffer.
5. Calculate and log the NUMA penalty percentage.

---

### Exercise 3: Simulating Cgroups v2 Memory Pressure & OOM Eviction
**Goal**: Create a script and C++ stress program that exercises cgroup memory limits.

**Requirements**:
1. Create a cgroup v2 directory under `/sys/fs/cgroup/faang_test`.
2. Set `memory.max` to `50M` (50 Megabytes) and `memory.swap.max` to `0`.
3. Add the test program PID to `/sys/fs/cgroup/faang_test/cgroup.procs`.
4. Program sequentially allocates memory in 5 MB chunks and writes to each page (`memset`).
5. Observe the kernel's response: check `dmesg` for `oom-kill` event, inspect `/sys/fs/cgroup/faang_test/memory.events` for `oom` and `oom_kill` counters.
