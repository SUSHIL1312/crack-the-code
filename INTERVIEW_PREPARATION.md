# 🎯 FAANG Interview Preparation: Fast-Track High-Yield Master Guide

> **Objective**: Prepare for technical interviews at Google, Meta, Amazon, Microsoft, Apple, Netflix, and Uber with **maximum efficiency** and **zero fluff**. This guide focuses exclusively on high-frequency, battle-tested concepts that interviewers actually evaluate.

---

## ⚡ The 3 Study Tracks

Choose your track based on the time you have before your interview:

```
┌────────────────────────────────────────────────────────────────────────┐
│  TRACK A: 7-Day High-Yield Crash Course                                │
│  Focus: High-frequency essentials & common traps. 4-6 hours/day.       │
├────────────────────────────────────────────────────────────────────────┤
│  TRACK B: 14-Day Intensive Interview Ready                             │
│  Focus: Core CS + Modern C++ / Python + REST API Design. 3-4 hours/day.│
├────────────────────────────────────────────────────────────────────────┤
│  TRACK C: 30-Day Zero to Hero Mastery                                  │
│  Focus: Full curriculum + All Capstone Projects + System Design.       │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 📅 Track A: 7-Day High-Yield Revision Schedule

| Day | Focus Area | High-Yield Topics (Must-Know) | Key Files to Review |
|---|---|---|---|
| **Day 1** | **OS: Processes & Concurrency** | Process vs Thread memory, PCB, Context Switch cost, Mutex vs Spinlock vs Semaphore, Deadlock (4 Coffman conditions). | `core-cs/operating-systems/01-processes-and-threads/`, `07-synchronization-primitives/` |
| **Day 2** | **OS: Virtual Memory & I/O** | Page tables, TLB, Page Fault handling, Copy-On-Write (`fork`), Non-blocking I/O (`epoll` vs `select`). | `core-cs/operating-systems/05-virtual-memory/`, `07-event-driven-epoll-server` |
| **Day 3** | **Networks: Transport & Web** | TCP 3-way handshake, TCP connection teardown & TIME_WAIT (2MSL), TCP Reno/Cubic Congestion Control, HTTP/1.1 vs HTTP/2 vs HTTP/3. | `core-cs/computer-networks/05-tcp-internals/`, `09-http-and-https/` |
| **Day 4** | **System Design: REST API Design** | Roy Fielding's 6 constraints, Safe vs Idempotent methods, Status codes (401 vs 403, 409, 412, 429), `Idempotency-Key` for payments, Cursor pagination. | `system-design/rest-api-design/` (All 5 chapters + `rest_api_server_demo.py`) |
| **Day 5** | **Language Core: Memory & Lifetime** | **C++**: RAII, Rule of 5, `std::move` vs `std::forward`, Smart pointers control blocks (`unique_ptr`, `shared_ptr`, `weak_ptr`).<br>**Python**: PyObject, Mutability, `__new__` vs `__init__`, Generational GC. | `languages/modern-cpp/02-raii-and-rule-of-five/`, `languages/python/02-cpython-internals-and-gc/` |
| **Day 6** | **Language Core: Concurrency** | **C++**: Atomics, acquire-release memory orderings, `std::condition_variable` spurious wakeups.<br>**Python**: GIL internals, CPU-bound vs I/O-bound, `asyncio` event loop. | `languages/modern-cpp/07-concurrency/`, `languages/python/03-gil-and-concurrency/` |
| **Day 7** | **Mock Interview & Traps Review** | Review common candidate traps, practice explaining "What happens when you type google.com in a browser?", solve 2 LLD/system design problems. | `ROADMAP.md`, `system-design/README.md` |

---

## 🧠 High-Yield Interview Cheat Sheet (The 20 Questions Asked in 90% of Interviews)

### 1. Operating Systems
1. **Process vs Thread**: A process has its own isolated virtual address space (Code, Data, Heap, Stack, FDs). Threads within the same process share Code, Data, Heap, and File Descriptors, but each thread has its own private Stack, Program Counter, and CPU Registers.
2. **Context Switching Cost**: Involves saving CPU register state, switching kernel stack, updating MMU page directory base register (`CR3` on x86, `TTBR0` on ARM), and invalidating/flushing TLB cache entries (expensive!).
3. **Deadlock Conditions (Coffman Criteria)**: Mutual Exclusion, Hold and Wait, No Preemption, Circular Wait. Breaking *any single condition* guarantees deadlock freedom.
4. **Virtual Memory & Page Faults**: MMU translates virtual address to physical address via Page Tables. If Present bit is 0, CPU raises a Page Fault interrupt. The OS kernel reads the missing page from disk swap/backing file into a free physical frame, updates the PTE, and resumes instruction.
5. **Why `fork()` is fast**: Copy-on-Write (COW). The kernel duplicates page table entries with write-protection flags rather than physically duplicating physical memory pages. Physical pages are only duplicated when either process attempts to write.

### 2. Computer Networks
6. **What happens when you type `https://google.com`?**:
   - Browser checks DNS cache (Browser -> OS -> Local DNS Resolver -> Root -> TLD -> Authoritative).
   - Resolves IP address.
   - TCP 3-Way Handshake (`SYN` -> `SYN-ACK` -> `ACK`).
   - TLS 1.3 Handshake (ClientHello + KeyShare -> ServerHello + KeyShare + Encrypted Certificate -> Finished).
   - HTTP/2 or HTTP/3 GET request sent over encrypted stream.
   - Server processes request and returns HTTP `200 OK` with HTML.
   - Browser parses HTML, builds DOM tree, downloads CSS/JS, and renders page.
7. **TCP vs UDP**: TCP is connection-oriented, byte-stream, reliable (ACKs, retransmissions), in-order, flow-controlled, congestion-controlled. UDP is connectionless, datagram-based, unreliable, out-of-order, lightweight (8-byte header vs 20-60 byte TCP header).
8. **Why `TIME_WAIT` state lasts 2MSL (Maximum Segment Lifetime)?**:
   - To ensure the final `ACK` sent by the closing host reaches the other side. If lost, the remote host retransmits `FIN`, which can only be acknowledged if the host is still in `TIME_WAIT`.
   - To allow all old, delayed packets in the network to expire before the same `(IP, Port)` pair is reused.
9. **TCP Congestion Control**: Slow Start (exponential window growth $2^N$), Congestion Avoidance (linear growth $W+1$), Fast Retransmit (3 duplicate ACKs), Fast Recovery.

### 3. REST API & System Design
10. **Safe vs Idempotent**: Safe methods (`GET`, `HEAD`) never modify server state. Idempotent methods (`GET`, `PUT`, `DELETE`) produce the same server state whether executed once or 10 times. `POST` is neither safe nor idempotent.
11. **How to Prevent Double Charging**: Mandate an `Idempotency-Key` UUID header on checkout `POST`. The server checks Redis atomically: if key was already processed, return cached response; if key is in flight, return `409 Conflict`; if new, lock key and process.
12. **Cursor vs Offset Pagination**: Offset pagination (`LIMIT 20 OFFSET 1000000`) forces the DB to read and discard 1,000,000 rows ($O(N)$). Cursor pagination (`WHERE id < cursor LIMIT 20`) uses direct B-tree index seek ($O(\log N)$) and avoids duplicate/skipped items during real-time writes.
13. **Preventing Lost Updates**: Use `ETag` and `If-Match`. If Client B sends an update with a stale ETag, server rejects with `412 Precondition Failed`.

### 4. Modern C++
14. **Rule of Five**: If a class defines a destructor, it almost certainly must define all 5: Destructor, Copy Constructor, Copy Assignment, Move Constructor, Move Assignment.
15. **`std::move` vs `std::forward`**: `std::move(x)` is an unconditional cast to an rvalue reference `T&&`. `std::forward<T>(x)` conditionally casts to rvalue reference *only* if the argument was passed as an rvalue (preserves value category in templates).
16. **`std::unique_ptr` vs `std::shared_ptr`**:
    - `unique_ptr`: Zero memory overhead (`sizeof == sizeof(void*)` with empty deleter via EBO), exclusive ownership, movable but non-copyable.
    - `shared_ptr`: 16 bytes on 64-bit systems (pointer to object + pointer to control block). Control block contains atomic strong ref count, atomic weak ref count, allocator, and custom deleter.
17. **Why Virtual Destructors are Mandatory**: Deleting a derived object through a pointer to base class without a virtual destructor causes **Undefined Behavior** (the derived destructor is never invoked, leaking derived resources).
18. **Memory Orderings**: `relaxed` (atomicity only, no ordering), `acquire-release` (synchronizes-with edge between store and load, zero overhead on x86 TSO), `seq_cst` (globally agreed-upon total ordering with hardware memory barriers).

### 5. Python
19. **The GIL (Global Interpreter Lock)**: A mutex that prevents multiple native threads from executing Python bytecodes simultaneously. Necessary because CPython's memory management (`ob_refcnt`) is not thread-safe. As a result, multithreading in CPython speeds up **I/O-bound** tasks, but does NOT speed up **CPU-bound** tasks (use `multiprocessing` instead).
20. **Generational Garbage Collection**: CPython uses **Reference Counting** for immediate deallocation when `refcnt == 0`. To break circular references (`A.b = B; B.a = A`), it runs a cyclic generational collector with 3 generations (Gen 0, Gen 1, Gen 2) based on survival thresholds.

---

## 🏢 Company-Specific Interview Focus

| Company | High-Yield Technical Focus | What Interviewers Emphasize |
|---|---|---|
| **Google** | Concurrency, Memory Models, Linux Syscalls, Clean Architecture | Algorithmic efficiency ($O(1)$/$O(\log N)$), zero-copy optimizations, race-condition freedom, edge cases. |
| **Meta** | High-Throughput I/O, Asyncio, REST API Design, Multiprocessing | Practical systems thinking, distributed caching, idempotency, horizontal scalability. |
| **Apple** | Modern C++ (Memory layout, RAII, Vtables), Kernel / POSIX primitives | Deep low-level knowledge, cache line alignment, CPU register usage, deterministic memory management. |
| **Amazon** | Scalability, Object-Oriented Design, Microservices, Error Handling | Leadership principles applied to code: resilience, fault tolerance, transaction rollback, clear API contracts. |
| **Netflix** | Concurrency, Resiliency, Distributed State, Rate Limiting | Chaos engineering mindset: handling downstream timeouts, circuit breakers, graceful degradation. |
