# Core Computer Science Curriculum

The **Core Computer Science** track provides a deep, systems-level understanding of how computer hardware, operating systems, and network communication operate together to execute software at scale.

In top-tier engineering interviews (Google, Meta, Amazon, Microsoft, Netflix, Apple, Uber, NVIDIA, Qualcomm), questions routinely pivot from basic algorithmic complexity to how memory is mapped, how the kernel handles concurrency, and how network packets traverse the protocol stack.

---

## 📂 Subsystems

```
core-cs/
├── operating-systems/       # 14 Deep-Dive Modules + 7 Capstone Projects (C++ / Linux)
└── computer-networks/       # 13 Deep-Dive Modules + 7 Capstone Projects (C++ / POSIX Sockets)
```

---

## 1. [Operating Systems](./operating-systems/)
Covers computer fundamentals, processes, threads, CPU scheduling, context switching, memory management, virtual memory, multi-level paging, synchronization primitives, deadlocks, multicore concurrency, Linux syscalls, file systems, IPC, and advanced container/cgroup internals.

* **Language**: Modern C++ (C++17/C++20) and POSIX / Linux System Calls.
* **Capstone Projects**: Mini Shell, Production Thread Pool, Thread-Safe Bounded Queue, Memory Allocator Simulator, Multi-Process Shared Memory Ring Buffer, Linux Process Monitor, and Event-Driven Epoll Server.

---

## 2. [Computer Networks](./computer-networks/)
Covers network models (OSI and TCP/IP), packet encapsulation, IP addressing & CIDR subnetting, Ethernet & ARP, transport layer mechanics, deep TCP internals (handshake, sliding window, flow & congestion control, teardown, TIME_WAIT), UDP & modern QUIC/HTTP/3, socket programming, DNS, HTTP/1.1 & HTTP/2, TLS 1.3 cryptography, and network performance tuning.

* **Language**: C++ Berkeley Sockets and Network Diagnostic Tools.
* **Capstone Projects**: TCP Chat Application, Multi-Client Chat Server, RFC-compliant HTTP Server from scratch, Simple HTTP Client, Non-blocking Event Loop Server, Multi-Threaded File Transfer, and Caching DNS Resolver.
