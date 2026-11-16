# Module 13: Advanced OS Concepts & Modern Linux Internals

## Overview
Modern cloud infrastructure, high-frequency trading platforms, and large-scale distributed systems rely on advanced operating system primitives. Container runtimes (Docker, containerd, Kubernetes), high-performance databases, and ultra-low-latency engines operate at the intersection of Linux namespaces, cgroups, NUMA hardware topologies, CPU cache hierarchies, and zero-syscall asynchronous I/O (`io_uring`).

In this module, you will master the internals of containerization, memory hierarchy optimization, and kernel resource control.

---

## Learning Objectives
1. **Linux Namespaces**: Deeply understand all 8 Linux namespaces (`PID`, `NET`, `MNT`, `UTS`, `IPC`, `USER`, `CGROUP`, `TIME`) and how they provide isolation without virtualization.
2. **Control Groups (cgroups v2)**: Learn how the unified cgroup tree regulates CPU shares, memory ceilings (`memory.max`), and blkio limits.
3. **Container Mechanics**: Dissect the exact sequence required to build a container from scratch using `unshare()`, `pivot_root()`, and `seccomp`.
4. **OOM Killer Subsystem**: Understand badness scoring, `/proc/[pid]/oom_score_adj`, and memory cgroup eviction algorithms.
5. **NUMA Architecture & CPU Cache Hierarchy**: Measure the latency penalty of cross-socket remote memory access vs local memory, and master cache associativity, TLB misses, and Huge Pages (2 MB / 1 GB).
6. **Next-Generation I/O (`io_uring`)**: Contrast traditional synchronous and readiness-based I/O (`epoll`) with submission/completion ring buffer architectures.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of container isolation, cgroups v2, NUMA, and memory hierarchies.
- `internal-working.md`: Kernel implementation details: `task_struct->nsproxy`, cgroup controllers, memory watermarks, and `io_uring` kernel polling.
- `visual-explanation.md`: Diagrams of the cgroup v2 tree, container namespace isolation, and NUMA node interconnects.
- `examples/`:
  - `container_namespaces_demo.cpp`: Demonstrates building isolated execution boundaries via `clone()` / `unshare()`.
  - `numa_and_cache_latency_bench.cpp`: Empirical pointer-chasing benchmark revealing L1, L2, L3, and main RAM latency cliffs.
- `exercises/`: Advanced systems engineering exercises.
- `solutions/`: Production solutions.
- `interview-questions.md`: FAANG interview questions (Docker internals, OOM handling in Kubernetes, NUMA tuning for database engines).
- `common-mistakes.md`: Frequent production pitfalls (cgroup v1 vs v2 differences, NUMA false sharing, memory limit thrashing).
