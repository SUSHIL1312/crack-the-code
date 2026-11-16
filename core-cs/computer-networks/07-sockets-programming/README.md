# Module 07: Sockets Programming & Berkeley API

## Overview
The Berkeley Sockets API (POSIX socket standard) is the universal application programming interface connecting user-space software to the kernel's network protocol stack. Whether writing high-performance web servers in C++, networking libraries in Rust, or database drivers in Python, every networked process ultimately interfaces with the operating system through socket descriptors.

In this module, you will master the socket lifecycle, address resolution with `getaddrinfo()`, socket configuration options (`SO_REUSEADDR`, `SO_REUSEPORT`, `TCP_NODELAY`), non-blocking socket manipulation with `fcntl()`, and robust error handling.

---

## Learning Objectives
1. **The POSIX Socket Lifecycle**:
   - Server: `socket()` $\to$ `setsockopt()` $\to$ `bind()` $\to$ `listen()` $\to$ `accept()` $\to$ `recv()`/`send()` $\to$ `close()`.
   - Client: `socket()` $\to$ `connect()` $\to$ `send()`/`recv()` $\to$ `close()`.
2. **Modern Address Resolution**: Replace deprecated `gethostbyname()` with protocol-independent, thread-safe `getaddrinfo()` and `struct sockaddr_storage`.
3. **Socket Configuration Options**:
   - Eliminate `EADDRINUSE` with `SO_REUSEADDR`.
   - Enable multi-threaded kernel port sharing with `SO_REUSEPORT`.
   - Disable Nagle's 200ms delay with `TCP_NODELAY`.
4. **Blocking vs Non-Blocking I/O**: Master `fcntl(fd, F_SETFL, O_NONBLOCK)` and handle `EAGAIN`, `EWOULDBLOCK`, and `EINTR`.
5. **Graceful Half-Close**: Use `shutdown(fd, SHUT_WR)` to signal EOF to the peer while continuing to read the response.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of sockets, file descriptor abstraction, address families, and socket options.
- `internal-working.md`: Socket buffer queues (`sk_buff`), `listen()` backlog queues (`syncookie` vs accept queue), and kernel state transitions.
- `visual-explanation.md`: Step-by-step sequence diagrams of socket syscall execution paths and backlog queues.
- `examples/`:
  - `robust_tcp_echo_suite.cpp`: Production C++ implementation demonstrating robust socket options, non-blocking mode, and half-close semantics.
- `exercises/`: Socket programming implementation challenges.
- `solutions/`: Reference production solutions.
- `interview-questions.md`: FAANG interview questions (e.g. `SO_REUSEPORT` kernel load balancing, `shutdown()` vs `close()`, handling `EINTR`).
- `common-mistakes.md`: Ignoring partial `send()`/`recv()`, forgetting `htons()`, leaking socket descriptors.
