# ⏱️ Last-Minute FAANG Interview Cram Sheet (60-Minute Rapid Revision)

> **How to use this guide**: Sitting outside the interview room or reviewing the evening before? This document strips away academic theory and gives you the exact high-yield answers, memory layouts, code idioms, and edge-case traps tested across **Google, Meta, Apple, Amazon, Netflix, and Uber**.

---

## 📑 60-Minute Fast Navigation

1. [Operating Systems (10 Mins)](#1-operating-systems-in-10-minutes)
2. [Concurrency & Synchronization (10 Mins)](#2-concurrency--synchronization-in-10-minutes)
3. [Computer Networks (10 Mins)](#3-computer-networks-in-10-minutes)
4. [REST API & System Design (10 Mins)](#4-rest-api--system-design-in-10-minutes)
5. [Modern C++ (10 Mins)](#5-modern-c-in-10-minutes)
6. [Python & OOP (10 Mins)](#6-python--oop-in-10-minutes)
7. [The Top 25 Rapid-Fire One-Liners](#7-the-top-25-rapid-fire-one-liners)

---

## 1. Operating Systems in 10 Minutes

### 🧠 Process vs Thread Memory Layout
```
Process (MMU Address Space)
├── Code (Text) Segment      ─── Shared across all threads
├── Data & BSS Segments       ─── Shared across all threads
├── Heap Memory               ─── Shared across all threads
├── Open File Descriptors     ─── Shared across all threads
├── Thread 1: Stack & Registers ─ PRIVATE (Stack pointer, PC, local vars)
└── Thread 2: Stack & Registers ─ PRIVATE (Stack pointer, PC, local vars)
```

### ⚡ Context Switch Cost
- **Thread Context Switch**: Save CPU registers, switch Kernel Stack pointer, restore registers. Takes **~100–300 nanoseconds**.
- **Process Context Switch**: Everything above PLUS:
  1. Reload MMU Page Table Base Pointer (`CR3` on x86, `TTBR0` on ARM).
  2. Flush / invalidate Translation Lookaside Buffer (TLB) cache.
  3. Memory cache misses (L1/L2 cache pollution). Takes **~1–2 microseconds** ($10\times$ slower!).

### 📦 Copy-On-Write (COW) `fork()`
- When `fork()` is called, the kernel **does NOT copy physical memory**.
- It duplicates the parent's page table entries and marks all pages as **Read-Only**.
- When either parent or child attempts to write, CPU raises a Page Fault interrupt. The kernel duplicates *only that single 4 KB page*, marks it writable, and resumes.

### 🗺️ Virtual Memory & Paging Walk
```
Virtual Address (48-bit on x86-64):
[PGD: 9 bits] -> [P4D: 9 bits] -> [PUD: 9 bits] -> [PMD: 9 bits] -> [PTE: 9 bits] -> [Physical Offset: 12 bits (4 KB)]
```
- **TLB (Translation Lookaside Buffer)**: Hardware L1 cache for address translation ($O(1)$, ~1 CPU cycle).
- **Page Fault**: Occurs when Present bit in PTE is 0. CPU jumps to kernel interrupt handler:
  1. If address is invalid $\to$ `SIGSEGV` (Segmentation fault).
  2. If valid but swapped to disk $\to$ Allocates physical RAM frame, reads page from disk, updates PTE Present bit to 1, resumes instruction.

### 📁 Inode vs File Descriptor
- **File Descriptor (FD)**: An integer index into the process's private open file table. Points to system-wide file description (offset, flags).
- **Inode**: Filesystem metadata structure stored on disk (size, permissions, timestamps, data block pointers). An inode has NO filename; directory entries (`dentry`) map `filename -> inode_number`.
- **Hard Link**: Another directory entry pointing to the same Inode (increases link count).
- **Soft Link (Symlink)**: A separate file whose content is the path string of the target.

---

## 2. Concurrency & Synchronization in 10 Minutes

### ⚖️ Mutex vs Semaphore vs Spinlock

| Primitive | Ownership? | Can Thread A lock & Thread B unlock? | Blocking Mechanism | Best Used For |
|---|:---:|:---:|---|---|
| **Mutex** | ✅ **YES** | ❌ **NO (Undefined Behavior)** | Puts thread to sleep via OS Futex (0% CPU) | Protecting critical sections (> 500 ns) |
| **Semaphore** | ❌ **NO** | ✅ **YES (Signaling Pattern)** | Puts thread to sleep via Kernel Queue (0% CPU) | Signaling, Resource Pooling (e.g. 10 DB connections) |
| **Spinlock** | ❌ NO | ✅ YES | Busy-wait loop burning **100% CPU** on core | Ultra-short waits (< 500 ns), Interrupt Handlers |

### 🚨 Why `wait()` MUST ALWAYS Be in a `while` Loop
```cpp
// ❌ REJECTION TRAP:
if (queue.empty()) cv.wait(lock);

// ✅ CORRECT:
while (queue.empty()) cv.wait(lock);
// Or: cv.wait(lock, [&]() { return !queue.empty(); });
```
**Why?**
1. **Spurious Wakeups**: OS signals (e.g. `SIGCHLD`) can interrupt the kernel `futex` sleep, waking the thread even if no signal was sent.
2. **Stolen Wakeups**: If Producer adds 1 item and wakes Consumer A, another Consumer B on a different CPU core can grab the mutex and pop the item first. Consumer A wakes up to an empty queue!

### 🔄 Producer-Consumer (Bounded Buffer) Golden Rule
- With **Condition Variables**: Use 2 CVs (`not_full`, `not_empty`) + 1 Mutex.
- With **Semaphores**: Use 2 Semaphores (`empty_slots = N`, `full_slots = 0`) + 1 Mutex.
- ⚠️ **Deadlock Trap**: Always acquire the **Semaphore BEFORE the Mutex**! If you lock the mutex first and wait on an empty slot, consumers can never acquire the mutex to free a slot $\to$ Permanent Deadlock!

### 💥 The 4 Coffman Deadlock Conditions
Deadlock occurs if and only if **all 4** hold simultaneously:
1. **Mutual Exclusion**: Non-shareable resource.
2. **Hold and Wait**: Holding one lock while waiting for another.
3. **No Preemption**: Cannot forcibly take lock from holding thread.
4. **Circular Wait**: $T_1 \to M_2 \to T_2 \to M_1 \to T_1$.
- **The #1 Fix in Production**: **Strict Global Lock Ordering** (Always lock resources by lower memory address first: `&a < &b`).

### 🛰️ Priority Inversion & Priority Inheritance (Mars Pathfinder Bug)
- **Problem**: Low-priority thread $T_L$ holds mutex. High-priority thread $T_H$ blocks waiting for mutex. Medium-priority thread $T_M$ preempts $T_L$. Result: $T_M$ indirectly starves $T_H$!
- **Solution (Priority Inheritance)**: Kernel temporarily boosts $T_L$'s priority to match $T_H$ until $T_L$ unlocks the mutex.

---

## 3. Computer Networks in 10 Minutes

### 🌐 "What happens when you type `https://google.com`?"
1. **DNS Resolution**: Browser Cache $\to$ OS Cache $\to$ Router / Local DNS Resolver $\to$ Root DNS (`.`) $\to$ TLD DNS (`.com`) $\to$ Authoritative DNS (`google.com`).
2. **TCP 3-Way Handshake**: Client sends `SYN (seq=x)` $\to$ Server sends `SYN-ACK (seq=y, ack=x+1)` $\to$ Client sends `ACK (ack=y+1)`.
3. **TLS 1.3 Handshake (1-RTT)**:
   - Client sends `ClientHello` + Supported Ciphers + Diffie-Hellman Key Share.
   - Server responds `ServerHello` + DH Key Share + Encrypted Certificate. Both compute shared symmetric session key.
4. **HTTP Request**: Client sends encrypted `GET / HTTP/2` request.
5. **Server Processing**: Reverse Proxy (Nginx/Envoy) terminates TLS $\to$ Load Balancer $\to$ Microservice backend $\to$ DB/Cache.
6. **HTTP Response**: Server sends `200 OK` with HTML.
7. **Browser Rendering**: Parses HTML $\to$ DOM tree; Parses CSS $\to$ CSSOM; Combines into Render Tree $\to$ Layout $\to$ Paint.

### ⏱️ Why TCP `TIME_WAIT` Lasts 2MSL (Maximum Segment Lifetime = ~60-120s)
1. **Ensure Final ACK Delivery**: If Client's final `ACK` is lost, Server retransmits `FIN`. Client must be alive in `TIME_WAIT` to send `ACK` again; otherwise Server receives `RST`!
2. **Drain Old Packets**: Prevents lingering, delayed duplicate packets from a previous connection from corrupting a new connection reusing the same `(IP, Port)` tuple.

### 🚀 HTTP Evolution Matrix

| Feature | HTTP/1.1 | HTTP/2 | HTTP/3 |
|---|---|---|---|
| **Transport** | TCP | TCP | **QUIC (over UDP)** |
| **Format** | Plain Text | Binary Framing | Binary Framing |
| **Multiplexing** | ❌ No (HOL blocking per TCP conn) | ✅ Yes (Streams over 1 TCP conn) | ✅ Yes (Independent QUIC streams) |
| **TCP HOL Blocking** | Yes | Yes (Packet loss stalls all streams) | ❌ **NO (Packet loss stalls only affected stream!)** |
| **Connection Migration**| ❌ No (Tied to IP:Port) | ❌ No | ✅ **YES (Tied to 64-bit Connection ID; WiFi $\to$ 5G seamless!)** |
| **Handshake Latency** | 1-RTT TCP + 2-RTT TLS | 1-RTT TCP + 1-RTT TLS | **0-RTT or 1-RTT total (Combined QUIC + TLS 1.3)** |

---

## 4. REST API & System Design in 10 Minutes

### 🔒 HTTP Verbs: Safety & Idempotency
- **Safe** ($f(x)$ does not mutate server state): `GET`, `HEAD`, `OPTIONS`.
- **Idempotent** ($f(f(x)) == f(x)$, calling multiple times produces identical state): `GET`, `PUT`, `DELETE`.
- **Non-Idempotent & Unsafe**: `POST`, `PATCH`.

### 💳 Preventing Double Charging (`Idempotency-Key`)
1. Client generates UUID: `Idempotency-Key: 9b1deb4d-...`
2. Server executes Redis atomic lock: `SET lock:key token NX EX 120`.
3. If Key exists with status `COMPLETED` $\to$ Return cached JSON response directly!
4. If Key exists with status `IN_FLIGHT` $\to$ Return `409 Conflict`.
5. If New $\to$ Process payment, persist in DB, save final response in Redis with 24h TTL, return `201 Created`.

### 📄 Pagination: Keyset/Cursor vs Offset
- **Offset** (`?limit=20&offset=1000000`): **Catastrophic Flaw**. Database must read and discard 1,000,000 rows from disk ($O(N)$). Shows duplicate items when rows are inserted.
- **Cursor** (`?limit=20&cursor=eyJpZCI6IDUwMDF9`): In SQL: `WHERE id < 5001 ORDER BY id DESC LIMIT 20`. Direct B-Tree index seek ($O(\log N)$). Zero duplicate items; lightning fast even on row #100,000,000.

### 🛡️ Optimistic Locking with ETags (Preventing Lost Updates)
1. Alice reads: `GET /orders/42` $\to$ Server returns `ETag: "v1"`.
2. Alice updates: `PATCH /orders/42` with `If-Match: "v1"` $\to$ Server updates and returns `ETag: "v2"`.
3. Bob attempts update with stale ETag: `PATCH /orders/42` with `If-Match: "v1"` $\to$ Server detects mismatch and rejects with **`412 Precondition Failed`**!

---

## 5. Modern C++ in 10 Minutes

### 🛡️ RAII & The Rule of 5 / 0
- **RAII**: Resource Acquisition Is Initialization. Acquire resource in Constructor, release in Destructor.
- **Rule of 5**: If you explicitly implement any one of these, implement all 5:
  1. Destructor `~Widget()`
  2. Copy Constructor `Widget(const Widget&)`
  3. Copy Assignment `Widget& operator=(const Widget&)`
  4. Move Constructor `Widget(Widget&&) noexcept`
  5. Move Assignment `Widget& operator=(Widget&&) noexcept`
- **Rule of 0**: Prefer designing classes using modern types (`std::string`, `std::vector`, `std::unique_ptr`) so the compiler generates all special members automatically.

### ⚡ `std::move` vs `std::forward`
- `std::move(x)`: **Unconditional cast** to an rvalue reference (`static_cast<T&&>(x)`). Moves resource ownership.
- `std::forward<T>(x)`: **Conditional cast**. Preserves value category (casts to rvalue *only* if the argument passed into a template function was an rvalue). Used in perfect forwarding.

### 📦 Smart Pointers Internals
- **`std::unique_ptr<T>`**: Zero memory overhead (`sizeof == sizeof(void*)`). Exclusive ownership. Move-only.
- **`std::shared_ptr<T>`**: 16 bytes (Raw pointer to object + Pointer to Control Block).
  - Control block contains: `strong_ref_count` (atomic), `weak_ref_count` (atomic), allocator, custom deleter.
  - `std::make_shared<T>()`: Allocates object + control block in **1 contiguous memory block** (cache-friendly, 1 allocation).
  - `std::shared_ptr<T>(new T)`: Performs **2 separate heap allocations**.
- **`std::weak_ptr<T>`**: Non-owning observer. Does not increment `strong_ref_count` (breaks circular references). Must call `.lock()` before accessing.

### 🎯 Dynamic Dispatch & Vtables
```
Heap Object:
[ *vptr (8 bytes) ] -> [ .rodata VTABLE ]
[ member_data...  ]     ├── &Base::func1() -> Address in .text
                        └── &Derived::func2() -> Address in .text
```
- **Assembly Call**: `call *0x8(%rax)` (indirect memory jump).
- **Mandatory Virtual Destructor Rule**: If a class has at least one virtual function, its destructor **MUST be virtual**! Deleting a derived object via a `Base*` without a virtual destructor causes **Undefined Behavior** (derived destructor never executes, leaking resources).
- **Object Slicing**: Passing a derived object by value (`void fn(Base b)`) slices away all derived members and copies only the `Base` slice. Always pass polymorphic objects by reference or pointer (`const Base&`).

---

## 6. Python & OOP in 10 Minutes

### 🔒 The GIL (Global Interpreter Lock)
- A mutex that prevents multiple OS threads from executing Python bytecodes simultaneously.
- **Why?** CPython's memory management (`ob_refcnt`) is not thread-safe.
- **Interview Rule**:
  - Python Multithreading speeds up **I/O-bound** tasks (GIL is released during network socket reads and disk I/O).
  - Python Multithreading does **NOT** speed up **CPU-bound** tasks (threads fight for GIL on 1 core; use `multiprocessing` instead).

### 🗑️ CPython Memory & Garbage Collection
1. **Reference Counting (Primary)**: Every object has `ob_refcnt`. When count drops to 0, memory is freed **immediately** ($O(1)$).
2. **Generational Cyclic GC (Secondary)**: Handles circular references (`a.cycle = b; b.cycle = a`). Runs across 3 generations (Gen 0, Gen 1, Gen 2) using tri-color marking. Surviving objects get promoted.

### 🏗️ `__new__` vs `__init__`
- `__new__(cls, ...)`: The **Allocator**. Static method that actually creates and returns the new object instance in memory. (Used to implement Singletons or customize immutable subclasses like `tuple`/`str`).
- `__init__(self, ...)`: The **Initializer**. Instance method that receives the already-allocated instance and initializes its attributes.

### 🧬 Method Resolution Order (MRO) & Cooperative `super()`
- Python resolves inheritance using the **C3 Linearization Algorithm**:
  1. Children precede parents.
  2. Direct parents appear in listed order (`class D(B, C)` $\to$ `B` before `C`).
  3. Every class appears only once.
- Always use `super().__init__()` instead of `Parent.__init__(self)` to guarantee cooperative lateral execution in diamond inheritance!

### 🏎️ `__slots__`: 70% Memory Optimization
- By default, Python instances store attributes in a dynamic dictionary `self.__dict__` (~152 bytes minimum overhead per object).
- Declaring `__slots__ = ('x', 'y')` replaces `__dict__` with a static C-level array of pointers. Reduces memory by **~70–86%** and accelerates attribute lookup!

---

## 7. The Top 25 Rapid-Fire One-Liners

1. **Can Thread A lock a Mutex and Thread B unlock it?**
   *No! A Mutex has ownership; doing this causes Undefined Behavior. Use a Semaphore instead.*
2. **What does `std::jthread` do that `std::thread` doesn't?**
   *Automatically joins in its destructor and supports cooperative cancellation via `std::stop_token`.*
3. **What is the difference between `realloc()` and allocating new memory?**
   *`realloc()` attempts to grow the buffer in-place; if adjacent memory is unavailable, it allocates a new block, copies bytes, frees the old block, and returns the new pointer.*
4. **Why is `sizeof` an empty struct in C++ 1 byte, but in C it is 0?**
   *In C++, every distinct object must have a unique memory address (`&a != &b`).*
5. **What is the cost of a virtual function call?**
   *One pointer indirection (`vptr -> vtable -> function pointer`) and missed compiler inline optimization opportunities.*
6. **What is False Sharing in multi-threading?**
   *When two independent variables accessed by different CPU cores reside on the same 64-byte Cache Line, causing CPU cores to invalidate each other's L1 cache. Fix with `alignas(64)`.*
7. **What is the difference between TCP Flow Control and Congestion Control?**
   *Flow Control protects the receiver (`rwnd`, sliding window). Congestion Control protects the intermediate network routers (`cwnd`, slow start/cubic).*
8. **Why does HTTP/2 still suffer from Head-of-Line (HOL) blocking?**
   *Because HTTP/2 multiplexes all streams over 1 TCP connection. If 1 TCP packet is dropped, the TCP stack stalls all multiplexed streams until retransmission succeeds.*
9. **How does HTTP/3 solve TCP HOL blocking?**
   *HTTP/3 runs over QUIC (UDP). Each stream has independent packet delivery; a dropped packet in Stream 1 never blocks Stream 2.*
10. **What is an Epoll / Event Loop?**
    *An $O(1)$ kernel I/O notification mechanism using a Red-Black tree of monitored FDs and a ready list populated by kernel NIC interrupts.*
11. **What is the difference between 401 Unauthorized and 403 Forbidden?**
    *401 means "Unauthenticated" (who are you? missing/invalid token). 403 means "Unauthorized" (I know who you are, but you don't have permission).*
12. **What status code should an API return on duplicate resource creation?**
    *`409 Conflict` (or `422 Unprocessable Entity` for validation errors).*
13. **Why should you never store tokens in URL query params?**
    *Query parameters are logged in plaintext across server access logs, CDN logs, browser histories, and Referer headers.*
14. **What is the ABA problem in lock-free programming?**
    *When a pointer value changes from A to B and back to A, tricking a CAS instruction into thinking nothing changed. Fix using Tagged Pointers / Version Counters.*
15. **What does `noexcept` on move constructors do?**
    *Enables `std::vector` to move elements during reallocation instead of copying them (required for the strong exception guarantee).*
16. **What is the difference between `fork()` and `vfork()`?**
    *`fork()` creates a child with Copy-On-Write pages. `vfork()` borrows the parent's address space directly and suspends the parent until child calls `execve()` or `_exit()`.*
17. **What happens if a process ignores `SIGKILL`?**
    *A process CANNOT ignore `SIGKILL` or `SIGSTOP`. They are handled directly by the kernel scheduler and immediately terminate the process.*
18. **What is a Zombie Process vs an Orphan Process?**
    *Zombie: Child has terminated, but parent hasn't called `wait()` to read exit code. Orphan: Parent died before child; child is adopted by `init` / `systemd` (PID 1).*
19. **What is Dijkstra's Banker's Algorithm?**
    *A deadlock avoidance algorithm that tests for safety by simulating the allocation of predetermined maximum possible amounts of all resources.*
20. **What is C++20 `std::string_view`?**
    *A non-owning, zero-allocation view of a string consisting simply of `const char* ptr` and `size_t length` (16 bytes).*
21. **What is the difference between Shallow Copy and Deep Copy?**
    *Shallow copy duplicates pointer addresses (sharing the underlying heap memory). Deep copy allocates new heap memory and duplicates the underlying data.*
22. **What is the difference between `is` and `==` in Python?**
    *`is` checks memory identity (`id(a) == id(b)`, pointer equality). `==` checks value equality (`a.__eq__(b)`).*
23. **Why are default arguments evaluated only once in Python?**
    *Function default arguments are evaluated at **function definition time**, not call time. Mutable defaults (`def fn(x=[])`) persist mutations across calls!*
24. **What is an Inverted Page Table?**
    *Instead of 1 page table per process, the OS maintains 1 global table indexed by physical frame numbers, mapping each frame to `(PID, Virtual Page Number)`.*
25. **What is the difference between Horizontal Scaling and Vertical Scaling?**
    *Vertical: adding more CPU/RAM to a single machine. Horizontal: adding more machines into a distributed pool behind a load balancer.*
