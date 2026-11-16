# Module 12: Inter-Process Communication (IPC) and Shared Memory

## Overview
Because operating systems isolate processes in separate virtual address spaces to ensure security and fault containment, processes cannot directly read or write each other's memory. Inter-Process Communication (IPC) provides kernel-mediated and hardware-assisted mechanisms for processes to coordinate, exchange data, and share state.

In this module, you will master the full spectrum of IPC mechanisms, from traditional pipes and FIFOs to high-throughput Unix Domain Sockets and zero-copy POSIX Shared Memory with process-shared synchronization.

---

## Learning Objectives
1. **IPC Mechanism Taxonomy**: Understand the trade-offs, throughput, latency, and synchronization requirements of Pipes, FIFOs, Shared Memory, Message Queues, and Unix Domain Sockets.
2. **Zero-Copy Data Transfer**: Master POSIX Shared Memory (`shm_open`, `mmap`) and how processes share physical RAM frames via separate page tables.
3. **Inter-Process Synchronization**: Implement process-shared mutexes (`PTHREAD_PROCESS_SHARED`), robust mutexes (`PTHREAD_MUTEX_ROBUST`), and POSIX semaphores (`sem_open`, `sem_wait`, `sem_post`).
4. **Unix Domain Sockets (`AF_UNIX`)**: Build high-performance bidirectional IPC with stream and datagram semantics, and pass open file descriptors between processes using `sendmsg` and `SCM_RIGHTS`.
5. **High-Performance IPC**: Implement a lock-free, single-producer single-consumer (SPSC) shared memory ring buffer achieving sub-microsecond latency.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of IPC mechanisms, performance characteristics, and kernel involvement.
- `internal-working.md`: Page table sharing under `mmap(MAP_SHARED)`, socket buffer queues (`sk_buff`), pipe ring buffers, and `SCM_RIGHTS` fd passing.
- `visual-explanation.md`: ASCII diagrams of memory layouts, IPC latency hierarchies, and ring buffer synchronization.
- `examples/`:
  - `shared_memory_ring_buffer.cpp`: Complete lock-free SPSC circular queue in shared memory.
  - `unix_domain_socket_ipc.cpp`: Echo server and client using bidirectional Unix domain stream sockets.
- `exercises/`: Challenging IPC implementation tasks.
- `solutions/`: Production-grade solutions and benchmarks.
- `interview-questions.md`: FAANG interview deep-dives (e.g., Chrome multi-process architecture IPC, shared memory vs socket throughput, fd passing).
- `common-mistakes.md`: Critical pitfalls like memory leaks in `/dev/shm`, deadlock when a process dies holding a process-shared mutex, and pipe deadlocks.
