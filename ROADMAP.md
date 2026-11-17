# 🗺️ Master Engineering Roadmap: Zero to Hero FAANG Interview Readiness

This roadmap provides an **authoritative, sequential path** to master Operating Systems, Computer Networks, Modern C++, Python, Object-Oriented Programming, and REST API Architecture. Follow this roadmap to systematically prepare for interviews at **Google, Meta, Apple, Amazon, Netflix, Uber, Bloomberg, and Microsoft**.

> [!TIP]
> **Sitting outside the interview room?**
> Jump directly to the **[60-Minute Last-Minute Revision Cram Sheet](LAST_MINUTE_REVISION.md)** for rapid-fire answers, memory diagrams, and high-frequency traps!
>
> **On a tight timeline?**
> Check out the **[7-Day, 14-Day, and 30-Day Fast Tracks](INTERVIEW_PREPARATION.md)**.

---

## 🧭 The 5-Phase Progression Blueprint

```mermaid
graph TD
    subgraph Phase 1: Core CS Foundations
        A1[OS: Memory, Paging & Process Architecture] --> A2[Networks: OSI, TCP Internals & Sockets]
    end

    subgraph Phase 2: Object-Oriented Programming Mastery
        B1[C++ OOP: Vtables, Object Slicing & Diamonds] --> B2[Python OOP: PyObject, C3 MRO & Slots]
    end

    subgraph Phase 3: Concurrency & Runtime Internals
        C1[Synchronization Masterclass: Futex, Mutex vs Semaphore] --> C2[C++ Atomics & Memory Model]
        C1 --> C3[Python GIL, Asyncio & Multiprocessing]
    end

    subgraph Phase 4: Distributed System Design & REST APIs
        D1[REST Design: Idempotency Keys, Cursor Pagination & ETags] --> D2[Runnable REST Reference Server]
    end

    subgraph Phase 5: Capstone Systems & FAANG Drills
        E1[Build 5 C++ / 4 Python Capstone Systems] --> E2[Company Question Banks & Mock Drills]
        E2 --> E3[Offer Ready 🚀]
    end

    Phase 1 --> Phase 2
    Phase 2 --> Phase 3
    Phase 3 --> Phase 4
    Phase 4 --> Phase 5
```

---

## 📌 Phase 1: Core CS Foundations (Operating Systems & Networks)

### 1. Operating Systems Internals
* **Key Modules**:
  * [OS 01: Processes & Threads](core-cs/operating-systems/01-processes-and-threads/) — Virtual memory layout (Stack $\downarrow$, Heap $\uparrow$), PCB, Context switch costs, `fork()` and Copy-on-Write (COW).
  * [OS 02: CPU Scheduling](core-cs/operating-systems/02-cpu-scheduling/) — Preemptive scheduling, Round-Robin simulator lab, CFS (Completely Fair Scheduler).
  * [OS 05: Virtual Memory](core-cs/operating-systems/05-virtual-memory/) — Multi-level page tables, TLB cache, Page Fault handling, Page replacement algorithms.
  * [OS 10: Linux Syscalls](core-cs/operating-systems/10-linux-internals-syscalls/) — User mode vs Kernel mode, syscall transition mechanics, `/proc` inspection.
  * [OS 12: IPC & Shared Memory](core-cs/operating-systems/12-ipc-and-shared-memory/) — Pipes, Unix Domain Sockets (`AF_UNIX`), POSIX Shared Memory (`shm_open`, `mmap`).

### 2. Computer Networks & Transport Layer
* **Key Modules**:
  * [Networks 01: OSI & TCP/IP Model](core-cs/computer-networks/01-osi-and-tcp-ip-model/) — Layer mapping, Packet Encapsulation pipeline (Data $\to$ Segment $\to$ Packet $\to$ Frame $\to$ Bits).
  * [Networks 05: TCP Internals](core-cs/computer-networks/05-tcp-internals/) — 3-way handshake sequence math, 4-way teardown, `TIME_WAIT` (2MSL), Reno/Cubic congestion control.
  * [Networks 07: Sockets Programming](core-cs/computer-networks/07-sockets-programming/) — Non-blocking sockets, `epoll`, `select`, TCP server from scratch.
  * [Networks 09: HTTP/HTTPS & Web Protocols](core-cs/computer-networks/09-http-and-https/) — HTTP/1.1 vs HTTP/2 vs HTTP/3 (QUIC), TLS 1.3 1-RTT handshake.

> [!NOTE]
> **Phase 1 Checkpoint**: Can you explain from scratch what happens when you type `https://google.com` into your browser, down to the socket, packet, MMU, and TLS level?

---

## 📌 Phase 2: Object-Oriented Programming (C++ & Python)

A unified deep dive comparing compiled, zero-overhead systems (C++) with dynamic interpreted runtimes (Python).

* **Curriculum Portal**: [`languages/oops/`](languages/oops/)
* **Modern C++ OOP ([`languages/oops/cpp/`](languages/oops/cpp/))**:
  * Vtable and `vptr` dynamic dispatch memory layout.
  * Assembly call breakdown (`call *0x8(%rax)`).
  * The mandatory virtual destructor rule (preventing undefined behavior memory leaks).
  * Object slicing traps and virtual constructor idiom (`clone()`).
  * Diamond inheritance, multiple `vptr`s, and `this` pointer adjustment offsets.
  * CRTP and C++20 Concepts (`requires`) for static polymorphism.
* **Python OOP ([`languages/oops/python/`](languages/oops/python/))**:
  * `PyObject` C header layout (`ob_refcnt`, `ob_type`).
  * `__new__` (Allocator) vs `__init__` (Initializer).
  * C3 Linearization algorithm and cooperative lateral `super()`.
  * `__slots__`: 70–86% memory reduction by bypassing `__dict__`.
  * Top 10 FAANG tricky interview questions in C++ and Python.

> [!NOTE]
> **Phase 2 Checkpoint**: Run the automated OOP test suites:
> ```bash
> cd languages/oops/cpp/code && make test
> cd ../../python/code && make test
> ```

---

## 📌 Phase 3: Systems Concurrency & Language Runtimes

* **Key Modules**:
  * **[🌟 Synchronization Masterclass](core-cs/operating-systems/07-synchronization-primitives/SYNCHRONIZATION_MASTERCLASS.md)**:
    * Thread synchronization vs Process synchronization (`PTHREAD_PROCESS_SHARED`).
    * The *Wait & Signal* paradigm (`wait()` / `notify()`, `sem_wait()` / `sem_post()`).
    * Why `wait()` MUST ALWAYS be enclosed in a `while` loop (Spurious wakeups, Stolen wakeups).
    * Mutex vs Semaphore vs Spinlock vs Futex vs Condition Variable vs Read-Write Lock.
    * Producer-Consumer Bounded Buffer with Condition Variables and Semaphores.
    * Priority Inversion (The Mars Pathfinder bug & Priority Inheritance).
    * 4 Coffman Deadlock Conditions & strict global lock ordering.
  * **Modern C++ Concurrency ([`languages/modern-cpp/07-concurrency-multithreading-and-atomics/`](languages/modern-cpp/07-concurrency-multithreading-and-atomics/))**:
    * `std::jthread` auto-joining and cooperative cancellation.
    * Hardware atomics and memory models: `relaxed`, `acquire-release`, `seq_cst`.
    * False Sharing and cache line bouncing (`alignas(64)`).
  * **Python Concurrency ([`languages/python/03-gil-and-concurrency/`](languages/python/03-gil-and-concurrency/))**:
    * The Global Interpreter Lock (GIL) mechanics.
    * Why multithreading does NOT speed up CPU-bound code in CPython.
    * Python 3.13 Free-threaded mode (PEP 703) and copy-and-patch JIT (PEP 744).
    * `asyncio` event loop and `TaskGroup` structured concurrency.

> [!NOTE]
> **Phase 3 Checkpoint**: Can you explain why acquiring a Mutex before a Semaphore in a Bounded Buffer causes a permanent deadlock?

---

## 📌 Phase 4: Distributed System Design & REST APIs

* **Key Modules**:
  * **[REST API Design Zero-to-Hero Guide](system-design/rest-api-design/)**:
    * Roy Fielding's 6 architectural constraints.
    * Safe vs Idempotent HTTP verbs.
    * **Payment Safety**: The `Idempotency-Key` header with Redis atomic lock flow (`SETNX`).
    * **Database Scale**: Keyset / Cursor pagination ($O(\log N)$) vs Offset pagination ($O(N)$ disk scan).
    * **Concurrency**: Optimistic locking with `ETag` and `If-Match` returning `412 Precondition Failed`.
    * **RFC 7807 Problem Details** standardized error schemas.
    * **Rate Limiting**: Token Bucket algorithm and standard HTTP headers (`RateLimit-Limit`, `429 Too Many Requests`).
    * **Runnable Server**: Execute the reference REST server in [`system-design/rest-api-design/examples/rest_api_server_demo.py`](system-design/rest-api-design/examples/rest_api_server_demo.py).

> [!NOTE]
> **Phase 4 Checkpoint**: How does Stripe handle an identical network retry on a mutating credit card charge endpoint?

---

## 📌 Phase 5: Capstone Systems & FAANG Question Banks

Prove your mastery by building and explaining production-grade capstone implementations:

1. **Operating Systems Capstones ([`core-cs/operating-systems/projects/`](core-cs/operating-systems/projects/))**:
   - Unix Shell from scratch with pipelines and signal handling.
   - Production Thread Pool with task queues and worker threads.
   - Event-driven Epoll Server handling 10,000 concurrent sockets.
2. **Modern C++ Capstones ([`languages/modern-cpp/projects/`](languages/modern-cpp/projects/))**:
   - Custom `shared_ptr` with atomic control block and custom deleters.
   - Dynamic `vector` with `std::move_if_noexcept` strong exception guarantee.
   - Concurrent LRU Cache with $O(1)$ reader-writer locking.
3. **Python Capstones ([`languages/python/projects/`](languages/python/projects/))**:
   - Token-bucket asynchronous rate limiter.
   - LRU Cache decorator with doubly linked list.
   - Thread-safe reusable connection pool.
4. **Company-Specific Question Banks**:
   - [Modern C++ FAANG Question Bank](languages/modern-cpp/09-high-frequency-interview-bank/)
   - [Python FAANG Question Bank](languages/python/06-high-frequency-interview-bank/)
   - [REST API Design Interview Questions](system-design/rest-api-design/interview-questions.md)

---

## ⏰ The 24-Hour Final Interview Countdown

| Time Before Interview | Action Item |
|---|---|
| **T-24 Hours** | Read through the **[Last-Minute Revision Cram Sheet](LAST_MINUTE_REVISION.md)** from top to bottom. |
| **T-12 Hours** | Review the **[Visual Architecture Gallery](README.md#%EF%B8%8F-visual-architecture-gallery-ai-generated-blueprints)** to refresh memory diagrams (Vtables, Process Address Space, Paging, TCP Lifecycle, Redis Idempotency). |
| **T-3 Hours** | Practice explaining the **Top 25 Rapid-Fire One-Liners** out loud. |
| **T-30 Minutes** | Skim the **Company-Specific Focus Table** in [INTERVIEW_PREPARATION.md](INTERVIEW_PREPARATION.md) to align with your interviewer's company mindset (Google, Meta, Apple, Netflix, or Amazon). |
