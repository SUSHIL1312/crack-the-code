# Capstone Project 05: Non-Blocking Event Loop Server

---

## 🎯 Overview

High-scale infrastructure software (such as Nginx, Redis, Envoy, and Node.js) relies on a **single-threaded non-blocking event loop** powered by operating system kernel demultiplexing primitives (`epoll` on Linux, `kqueue` on macOS/FreeBSD).

This project constructs an industrial-grade non-blocking event loop server from scratch:
1. **OS Demultiplexer Abstraction**: Clean cross-platform wrapper around `kqueue` (macOS) and `epoll` (Linux).
2. **Asynchronous Non-Blocking Sockets**: Enforces `O_NONBLOCK` on all descriptors, ensuring the event loop thread never blocks on I/O.
3. **Outbound Queue State Machine**: Handles partial writes and registers write readiness (`EVFILT_WRITE` / `EPOLLOUT`) only when kernel buffers are congested.
4. **Stress Benchmark**: Demonstrates hundreds of thousands of requests per second with sub-millisecond latencies.

---

## 🚀 Build and Run

```bash
make
make run
```
