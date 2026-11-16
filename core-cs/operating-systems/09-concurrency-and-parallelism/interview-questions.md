# Interview Questions: Concurrency & Multicore Systems

---

## Question 1: What is False Sharing? How do you detect and resolve it in C++?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Multicore Architecture & Performance
* **Classification**: Officially documented interview topic
* **Role Relevance**: High-Performance C++, Systems Engineers, Low-Latency Trading
* **Prerequisites**: CPU Caches, Cache lines, MESI protocol
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2–3 minute explanation covering cache line granularity, invalidation traffic, and alignment tools.

### Model Answer
**False Sharing** is a hardware performance degradation that occurs when two independent threads running on different CPU cores modify completely separate logical variables that happen to share the same physical **64-byte CPU cache line**.

1. **The Root Mechanism**:
   - The hardware cache controller maintains cache coherence at the granularity of **cache lines (64 bytes)**, not individual bytes or words.
   - When Core 0 writes to Variable A, the MESI protocol invalidates the entire 64-byte line across all other CPU cores.
   - When Core 1 attempts to write to Variable B (which is in the same line), Core 1 suffers a cache miss, stalls its execution pipeline, fetches the line from Core 0 over the interconnect bus, and invalidates Core 0's cache!
   - This "cache line ping-ponging" across the memory bus reduces throughput by $4\times$ to $10\times$.
2. **How to Detect It**:
   - Use Linux `perf c2c` (Cache-to-Cache analyzer) or Intel VTune to detect high rates of Remote HITM (Hit Modified cache line) stalls.
3. **How to Fix It**:
   - Pad and align variables to cache line boundaries:
     ```cpp
     // C++17 Standard approach:
     struct alignas(std::hardware_destructive_interference_size) ThreadData {
         uint64_t counter;
     };
     ```
   - Or explicitly align to 64 bytes: `alignas(64)`.

---

## Question 2: What is the ABA Problem in lock-free programming, and how do you solve it?

* **Difficulty**: Hard
* **Importance**: Critical
* **Category**: Lock-Free Data Structures & Atomics
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Engineers, Core Infrastructure, Database Kernel Developers
* **Prerequisites**: Compare-and-Swap (CAS), Lock-free Stack
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3-minute explanation tracing pointer recycling and generation counter resolution.

### Model Answer
The **ABA Problem** occurs in multithreaded lock-free data structures that rely on atomic **Compare-And-Swap (`CAS`)**:

1. **The Anomaly**:
   - A thread reads memory location $X$ and observes value `A`.
   - The thread is preempted. While asleep, other threads modify $X$ from `A` to `B`, and then modify it back to `A` (often because the memory allocator recycled the exact same memory address for a newly allocated node).
   - The original thread resumes and performs `CAS(&X, A, NewVal)`.
   - The CPU sees that $X$ still contains `A`. The CAS succeeds!
   - However, the underlying logical state of the data structure has changed completely, and the pointer references stale or recycled nodes, leading to **memory corruption or use-after-free**.
2. **How to Solve It**:
   - **Tagged Pointers / Double-Word CAS (DCAS)**: Pair the pointer with a monotonic revision counter. On 64-bit systems, pack a 48-bit pointer and a 16-bit counter into a single 64-bit word, or use 128-bit atomic CAS (`CMPXCHG16B` on x86-64).
   - **Hazard Pointers**: Threads publish the memory addresses they are actively reading into a thread-local hazard pointer list. Deallocating threads are forbidden from reclaiming memory while it is listed in any active hazard pointer.
   - **Epoch-Based Reclamation (EBR)**: Defers memory deallocation until all active threads have progressed past the current global epoch.
