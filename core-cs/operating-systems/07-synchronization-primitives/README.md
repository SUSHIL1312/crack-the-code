# Module 07: Synchronization Primitives & Thread Safety

Concurrency without synchronization causes race conditions, corrupted memory, and non-deterministic crashes. This module is one of the highest-priority interview subjects across FAANG engineering loops.

---

## 📑 Module Contents

1. [Theory & Concurrency Primitives](theory.md)
   * The Anatomy of a Critical Section & Mutual Exclusion (Mutual Exclusion, Progress, Bounded Waiting).
   * Primitive Catalog: Mutex, Spinlock, Counting Semaphore, Condition Variable, Read-Write Lock (`shared_mutex`), Barrier.
   * Busy-Waiting vs Blocking Synchronization trade-off curves.
   * Lock Contention and Fine-Grained vs Coarse-Grained Locking.
2. [Internal Working](internal-working.md)
   * How a Mutex actually works inside the kernel: The Linux **Futex (Fast Userspace Mutex)**.
   * Hardware Atomic Instructions: Compare-And-Swap (`CMPXCHG`), Test-And-Set (`XCHG`), Memory Fences.
   * Spurious Wakeups in Condition Variables (Why `while` loop is mandatory).
3. [Visual Explanations](visual-explanation.md)
   * Critical section timeline showing atomicity violation.
   * Linux Futex fast-path vs slow-path flow.
   * Reader-Writer lock states and starvation graphs.
4. [Examples & Code](examples/)
   * `race_condition_vs_mutex.cpp`: Replicating counter corruption without synchronization vs fixing it with `std::mutex` and `std::lock_guard`.
   * `custom_spinlock.cpp`: Custom user-space Spinlock implemented via `std::atomic_flag` with exponential backoff (`_mm_pause`).
   * `producer_consumer_condvar.cpp`: Production-grade Thread-Safe Bounded Buffer using `std::condition_variable`.
   * `reader_writer_lock.cpp`: Read-heavy benchmark with `std::shared_mutex` (shared locks for readers, unique locks for writers).
   * `dining_philosophers.cpp`: Reproducing and solving Dijkstra's Dining Philosophers problem with `std::scoped_lock` (deadlock-free).
5. [Exercises & Solutions](exercises/exercises.md)
   * Implementing a Counting Semaphore from scratch, building a thread-safe barrier.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Crucial FAANG questions: Mutex vs Semaphore, Futex mechanics, Spurious wakeups, Lock-free vs Locking.
7. [Common Mistakes](common-mistakes.md)
   * Using `if` instead of `while` with condition variables, holding locks during I/O, deadlocks from inverted lock hierarchy.
