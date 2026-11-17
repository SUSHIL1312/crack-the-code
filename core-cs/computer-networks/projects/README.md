# Computer Networks Capstone Projects

<!-- Chapter Navigation Header -->
| ⬅️ [Module 12: Network Programming](../12-network-programming/README.md) | 🏠 [Computer Networks Hub](../README.md) | End of Track 🏁 |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

This directory contains 7 production-grade, hands-on networking projects designed to solidify systems programming and low-level networking concepts.

Each project is self-contained with its own source code, build configuration (`Makefile`), and automated verification tests.

---

## 📋 Capstone Projects Overview

| # | Project Name | Directory | Key Concepts & Architecture |
| :--- | :--- | :--- | :--- |
| **01** | **TCP Chat Application** | [`01-tcp-chat-app`](./01-tcp-chat-app) | Binary protocol framing, length-prefixed frames, stream defragmentation, POSIX sockets |
| **02** | **Multi-Client Chat Server** | [`02-multiclient-chat-server`](./02-multiclient-chat-server) | Thread pool architecture, thread-safe client registry, broadcast channels, atomic session counters |
| **03** | **HTTP/1.1 Server From Scratch** | [`03-http-server-from-scratch`](./03-http-server-from-scratch) | RFC 9112 parser, HTTP methods (GET/POST), status codes, header parsing, Content-Length, Keep-Alive |
| **04** | **Low-Level HTTP Client** | [`04-simple-http-client`](./04-simple-http-client) | Socket-level HTTP/1.1 client, URL parsing, Chunked Transfer Decoding, redirect handling |
| **05** | **Non-Blocking Event Loop Server** | [`05-nonblocking-eventloop-server`](./05-nonblocking-eventloop-server) | Reactor pattern, OS multiplexing (`kqueue` on macOS / `epoll` on Linux), non-blocking socket state machines |
| **06** | **Multithreaded Chunked File Transfer** | [`06-multithreaded-file-transfer`](./06-multithreaded-file-transfer) | Reliable chunked protocol, sliding window transmission, SHA-256 / CRC32 checksum verification |
| **07** | **High-Performance DNS Resolver & Cache** | [`07-dns-lookup-cache`](./07-dns-lookup-cache) | RFC 1035 wire-format binary parser, LRU eviction, TTL expiration, recursive resolution simulation |

---

## 🛠️ Build and Test All Projects

Each project directory includes a `Makefile`:
```bash
# Build and run a specific project (e.g. Project 1)
cd 01-tcp-chat-app
make
make run
```

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 12: Network Programming](../12-network-programming/README.md) | [Computer Networks Hub](../README.md) | Track Completed 🏁 |
<!-- /Chapter Navigation Footer -->

