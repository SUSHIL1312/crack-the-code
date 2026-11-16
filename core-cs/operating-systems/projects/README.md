# Operating Systems: Capstone Projects

Hands-on, production-grade systems software projects designed to demonstrate mastery of OS concepts in engineering interviews at Google, Meta, Apple, Netflix, Uber, and systems infrastructure teams.

---

## Project Catalog

| Project | Key Concepts | Directory |
| :--- | :--- | :--- |
| **01. Mini Unix Shell** | Process lifecycle, `fork()`, `execvp()`, pipes (`pipe`, `dup2`), I/O redirection (`<`, `>`), job control (`&`), signal handling (`SIGINT`, `SIGCHLD`) | [`01-mini-shell/`](./01-mini-shell/) |
| **02. Production C++20 Thread Pool** | Concurrency, thread pools, task queue, condition variables, `std::packaged_task`, `std::future`, graceful shutdown | [`02-thread-pool/`](./02-thread-pool/) |
| **03. Thread-Safe Bounded Queue** | Synchronization, producer-consumer pattern, blocking wait, timeout support, poison pill shutdown | [`03-thread-safe-bounded-queue/`](./03-thread-safe-bounded-queue/) |
| **04. Memory Allocator Simulator** | Free-list allocator, boundary tags, memory coalescing, splitting, first-fit / best-fit, heap alignment | [`04-memory-allocator-simulator/`](./04-memory-allocator-simulator/) |
| **05. Multiprocess Producer-Consumer** | POSIX shared memory (`mmap`, `MAP_SHARED`), POSIX semaphores (`sem_open`, `sem_wait`, `sem_post`), zero-copy IPC | [`05-multiprocess-producer-consumer/`](./05-multiprocess-producer-consumer/) |
| **06. Linux Process & System Monitor** | Linux `/proc` filesystem parsing, PID inspection, CPU calculation, RSS/VMS tracking, file descriptor metrics | [`06-linux-process-monitor/`](./06-linux-process-monitor/) |
| **07. Event-Driven Non-Blocking Server** | I/O multiplexing, non-blocking sockets (`O_NONBLOCK`), `epoll` / `kqueue` / `poll` event loop architecture | [`07-event-driven-epoll-server/`](./07-event-driven-epoll-server/) |

---

## Build and Run Instructions

Every project includes a dedicated `Makefile` and can also be built using the top-level repository `Makefile` or CMake:

```bash
# Build a specific project
cd 01-mini-shell && make && ./mini_shell
cd ../02-thread-pool && make && ./thread_pool_test
```
