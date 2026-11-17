# Module 07: Synchronization Primitives & Thread Safety

Concurrency without synchronization causes race conditions, corrupted memory, and non-deterministic crashes. This module is one of the highest-priority interview subjects across FAANG engineering loops.

> [!TIP]
> **🚀 Comprehensive Interview Masterclass**: Check out the **[Synchronization Masterclass](SYNCHRONIZATION_MASTERCLASS.md)** covering thread vs process synchronization, the wait & signal paradigm, mutex vs semaphore vs spinlock, and high-frequency FAANG concurrency traps!

---

## 📑 Module Contents

0. **[🌟 Synchronization Masterclass (Interview Complete Guide)](SYNCHRONIZATION_MASTERCLASS.md)**
   * Complete end-to-end guide: Dijkstra's 3 criteria, Thread sync vs Process sync, Wait/Signal mechanics, Spurious wakeups, Producer-Consumer with CondVars & Semaphores, and top FAANG interview questions.
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
   * `producer_consumer_condvar.cpp`: Production-grade Bounded Buffer using `std::condition_variable` and `std::unique_lock`.
   * `producer_consumer_semaphores.cpp`: Canonical Bounded Buffer using C++20 `std::counting_semaphore`.
   * `producer_consumer_demo.py`: Python Bounded Buffer with `threading.Condition` and `threading.Semaphore`.
   * `thread_vs_process_sync.cpp`: Side-by-side comparison of Thread Sync (`std::mutex`) vs Process Sync across `fork()` (`PTHREAD_PROCESS_SHARED` + `mmap`).
   * `race_condition_vs_mutex.cpp`: Replicating counter corruption without synchronization vs fixing it with `std::mutex`.
   * `custom_spinlock.cpp`: Custom user-space Spinlock implemented via `std::atomic_flag` with exponential backoff (`_mm_pause`).
   * `reader_writer_lock.cpp`: Read-heavy benchmark with `std::shared_mutex` (shared locks for readers, unique locks for writers).
   * `dining_philosophers.cpp`: Reproducing and solving Dijkstra's Dining Philosophers problem with `std::scoped_lock` (deadlock-free).
5. [Exercises & Solutions](exercises/exercises.md)
   * Implementing a Counting Semaphore from scratch, building a thread-safe barrier.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Crucial FAANG questions: Mutex vs Semaphore, Futex mechanics, Spurious wakeups, Lock-free vs Locking.
7. [Common Mistakes](common-mistakes.md)
   * Using `if` instead of `while` with condition variables, holding locks during I/O, deadlocks from inverted lock hierarchy.
