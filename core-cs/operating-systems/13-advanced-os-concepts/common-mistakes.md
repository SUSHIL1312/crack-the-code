# Common Mistakes in Advanced OS Concepts & Linux Internals

### 1. Hardcoding Cgroups v1 File Paths in Modern Linux
- **Mistake**: Writing scripts or software that inspects `/sys/fs/cgroup/cpu/cpu.cfs_quota_us` or `/sys/fs/cgroup/memory/memory.limit_in_bytes`.
- **Reality**: Modern Linux distributions (Ubuntu 22.04+, RHEL 9+, Debian 11+, Kubernetes 1.25+) default to **cgroups v2**, which uses a unified hierarchy. The old paths do not exist.
- **Fix**: Check which version is mounted via `/sys/fs/cgroup/cgroup.controllers`. For v2, read `cpu.max` and `memory.max` under `/sys/fs/cgroup/...`.

---

### 2. Believing `docker run --memory 1G` Restricts Virtual Memory (VSZ)
- **Mistake**: Setting a container memory limit of 1 GB and assuming that a process calling `malloc(10 * 1024 * 1024 * 1024)` (10 GB) will immediately fail.
- **Reality**: Cgroup memory limits constrain **Resident Set Size (RSS)** and page cache, not Virtual Memory Size (VSZ). In Linux, optimistic overcommit (`vm.overcommit_memory = 0`) permits allocating 10 GB of address space. The OOM killer only triggers when the process actually **touches** more than 1 GB of physical memory pages.

---

### 3. Misunderstanding PID 1 Signal Handling in Containers
- **Mistake**: Running a custom binary directly as PID 1 in a container (`ENTRYPOINT ["/my_server"]`) without registering signal handlers, expecting `docker stop` to cleanly shut it down.
- **Reality**: The Linux kernel treats PID 1 specially: signals sent to PID 1 that do not have explicit custom signal handlers registered are **discarded**, even `SIGTERM`! As a result, `docker stop` waits 10 seconds, times out, and forcefully terminates the container with `SIGKILL`, leading to data corruption.
- **Fix**: Register explicit `SIGTERM`/`SIGINT` handlers in PID 1 code, or use a lightweight init process like `tini` (`docker run --init`).

---

### 4. Ignoring Cross-NUMA False Sharing on Multi-Socket Systems
- **Mistake**: Sharing a global atomic counter across threads running on different physical CPU sockets.
- **Reality**: When Core 0 on Socket 0 increments the counter, hardware cache coherency broadcasts an invalidation across the inter-socket interconnect (UPI/Infinity Fabric) to Socket 1. Socket 1 must stall while fetching the modified cache line across the interconnect, reducing overall multi-threaded throughput to slower than a single thread!
- **Fix**: Use per-thread or per-NUMA-node local counters, and periodically sum them up (like Linux `this_cpu_inc` or Java `LongAdder`).
