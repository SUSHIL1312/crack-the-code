# Module 12: Network Programming, I/O Multiplexing & High-Performance Concurrency

<!-- Chapter Navigation Header -->
| ⬅️ [Module 11: Network Performance](../11-network-performance/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Networks Capstone Projects](../projects/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Welcome to **Module 12** of the Computer Networks Curriculum. This module demystifies how high-performance internet servers (such as Nginx, Redis, Envoy, Netty, Node.js) scale to hundreds of thousands of concurrent connections using non-blocking I/O and event-driven architectures.

---

## 🎯 Learning Objectives

By the end of this module, you will be able to:
1. Explain the fundamental limitations of the classic **Thread-per-Connection** concurrency model and how the **C10K Problem** transformed systems architecture.
2. Differentiate between synchronous blocking, synchronous non-blocking, I/O multiplexing (`select`, `poll`, `epoll`, `kqueue`), and true asynchronous I/O (`io_uring`).
3. Master **Level-Triggered (LT)** vs **Edge-Triggered (ET)** event readiness notifications and prevent starvation and lost wakeups.
4. Architect the **Reactor Pattern** and **Proactor Pattern** with non-blocking socket state machines.
5. Compile and run a high-performance cross-platform Reactor event loop server.

---

## 📁 Module Structure

```
12-network-programming/
├── README.md                 # This orientation document
├── theory.md                 # I/O models, C10K, select/poll/epoll/kqueue, Reactor vs Proactor
├── internal-working.md       # Kernel readiness queues, RB-trees, waitqueues, ET vs LT mechanics
├── visual-explanation.md     # ASCII diagrams of event loops, edge vs level triggering
├── examples/
│   ├── reactor_event_loop.cpp # Cross-platform non-blocking Reactor event loop server
│   └── Makefile              # Build automation
├── exercises/
│   └── exercises.md          # Practical exercises on non-blocking I/O and event loops
├── solutions/
│   └── solutions.md          # Detailed engineering solutions and proofs
├── interview-questions.md    # Top FAANG systems architecture interview questions
└── common-mistakes.md        # Critical pitfalls in non-blocking event loops
```

---

## 🚀 Quick Start: Building and Running Examples

```bash
cd core-cs/computer-networks/12-network-programming/examples
make run
```

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 11: Network Performance](../11-network-performance/README.md) | [Computer Networks Hub](../README.md) | **[Networks Capstone Projects](../projects/README.md)** |
<!-- /Chapter Navigation Footer -->

