# Project 07: Event-Driven Non-Blocking Server (`epoll` / `kqueue`)

A production-grade, single-threaded event-driven TCP server implementing the Reactor pattern using OS I/O multiplexing (`epoll` on Linux, `kqueue` on macOS/FreeBSD).

---

## Architectural Highlights
- **Reactor Pattern**: Single-threaded event loop demultiplexing I/O events across thousands of concurrent connections without thread context-switching overhead.
- **Edge-Triggered (ET) vs Level-Triggered (LT)**:
  - Demonstrates non-blocking socket configuration (`fcntl(fd, F_SETFL, O_NONBLOCK)`).
  - Handles Edge-Triggered draining loop: reading in a loop until `read()` returns `-1` with `errno == EAGAIN` or `EWOULDBLOCK`.
- **Cross-Platform Portability**:
  - Automatically compiles using **Linux `epoll`** (`epoll_create1`, `epoll_ctl`, `epoll_wait`) when running on Linux.
  - Automatically compiles using **Darwin `kqueue`** (`kqueue`, `kevent`) when running on macOS.
- **Zero-Copy Buffering**: Dynamic per-connection buffer state managing partial reads and partial writes.

---

## Build and Run
```bash
make
./event_server
```
