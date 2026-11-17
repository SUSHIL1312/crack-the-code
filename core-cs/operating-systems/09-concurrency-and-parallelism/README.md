# Module 09: Concurrency, Parallelism & Multicore Systems

<!-- Chapter Navigation Header -->
| ⬅️ [Module 08: Deadlocks](../08-deadlocks/README.md) | 🏠 [Operating Systems Hub](../README.md) | [Module 10: Linux Syscalls](../10-linux-internals-syscalls/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Writing software that scales across dozens of CPU cores requires moving past high-level thread abstractions to understand hardware cache coherence protocols, memory bus contention, false sharing, and lock-free atomic primitives.

---

## 📑 Module Contents

1. [Theory & Multicore Architecture](theory.md)
   * Concurrency vs Parallelism.
   * Amdahl's Law & Gustafson's Law: The mathematical limits of multi-core scaling.
   * Multicore Cache Architecture: L1/L2 Private vs L3 Shared.
   * Cache Coherence Protocols (MESI / MOESI).
   * False Sharing & Cache Line Bouncing.
   * Lock-Free vs Wait-Free vs Blocking algorithms.
2. [Internal Working](internal-working.md)
   * The MESI protocol state transitions (Modified, Exclusive, Shared, Invalid).
   * Hardware Atomic Instructions: `CMPXCHG`, `XADD`, Store Buffers & Invalidate Queues.
   * The ABA Problem in Lock-Free structures: Pointer reuse hazard.
   * Tagged Pointers / Generation Counting resolution.
3. [Visual Explanations](visual-explanation.md)
   * False sharing diagram showing cache line ping-ponging across Core 0 and Core 1.
   * Amdahl's Law asymptotic speedup ceiling curve.
   * The ABA Problem pointer mutation timeline.
4. [Examples & Code](examples/)
   * `false_sharing_benchmark.cpp`: Empirical benchmark demonstrating a $6\times$ performance collapse caused by false sharing vs fixing it with `alignas(64)`.
   * `lock_free_stack.cpp`: Complete, working lock-free concurrent Treiber Stack in C++ using atomic Compare-And-Swap (`compare_exchange_weak`).
5. [Exercises & Solutions](exercises/exercises.md)
   * Amdahl's law scaling calculations, designing lock-free tagged pointers.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Essential FAANG questions: What is false sharing? Concurrency vs Parallelism, ABA problem, MESI protocol.
7. [Common Mistakes](common-mistakes.md)
   * Assuming atomic operations are free, ignoring memory bus saturation under high core counts.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 08: Deadlocks](../08-deadlocks/README.md) | [Operating Systems Hub](../README.md) | **[Module 10: Linux Syscalls](../10-linux-internals-syscalls/README.md)** |
<!-- /Chapter Navigation Footer -->

