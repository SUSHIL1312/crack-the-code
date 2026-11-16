# Common Mistakes & Traps: Concurrency & Multicore Systems

---

## 1. Believing "Lock-Free" Automatically Means Faster

### The Misconception:
"Lock-free algorithms don't use OS locks, so they are always faster than mutex-based data structures."

### The Reality:
Under high thread contention:
- In a lock-free queue or stack, $N$ threads execute atomic `compare_exchange_weak()` loops competing for the same memory word.
- Only **one thread succeeds** per CAS loop; the remaining $N-1$ threads fail and retry.
- Each failed CAS loop issues broadcast invalidations across the CPU memory bus, saturating the interconnect and causing extreme cache-line contention.
- In contrast, a well-tuned mutex-based queue puts waiting threads to sleep on a kernel wait queue, freeing the CPU cores and memory bus entirely!
- **Rule**: Lock-free structures excel under low-to-moderate contention or when threads cannot afford OS scheduling jitter (e.g. audio threads, interrupt handlers). Under heavy contention, partitioned/fine-grained locks or work-stealing queues often outperform naive lock-free CAS loops.

---

## 2. Using Default `std::memory_order_seq_cst` Everywhere

### The Performance Cost:
In C++, `std::atomic` operations without an explicit memory order parameter default to `std::memory_order_seq_cst` (Sequential Consistency):
- On x86-64, sequential consistency for loads is free, but atomic stores require an expensive `mfence` instruction or `xchg` to drain the hardware store buffer.
- On ARM64, sequential consistency requires full `dmb` (Data Memory Barrier) or `ldar`/`stlr` barrier instructions, stalling the out-of-order execution pipeline.
- Where appropriate, understanding **Acquire-Release semantics (`memory_order_acquire`, `memory_order_release`)** allows compilers and CPU pipelines to eliminate unnecessary hardware fences while still guaranteeing correct synchronization.
