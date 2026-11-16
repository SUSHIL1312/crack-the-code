# Exercises: Synchronization Primitives

---

## 🟢 Beginner Level

### Exercise 1.1: Implement a Counting Semaphore from Scratch
While modern C++20 provides `std::counting_semaphore`, implementing it using C++11/17 primitives is a classic FAANG interview question.
1. Implement a class `CountingSemaphore`:
   ```cpp
   class CountingSemaphore {
   public:
       explicit CountingSemaphore(int initial_count);
       void acquire(); // Decrements count; blocks if count <= 0
       void release(); // Increments count; notifies one waiter
   };
   ```
2. Use a `std::mutex`, `std::condition_variable`, and an integer `count_`.
3. Explain why the condition variable `wait()` predicate MUST check `count_ > 0`.
4. Demonstrate its usage by limiting concurrent access to a simulated connection pool to 3 threads.

---

## 🟡 Intermediate Level

### Exercise 2.1: Writer-Preference Read-Write Lock
Standard read-write locks (`std::shared_mutex`) can starve writers if an endless stream of readers keeps arriving.
1. Design and implement a `WriterPreferredRWLock` in C++:
   - Tracks `active_readers`, `active_writers`, and `waiting_writers`.
   - If any writer is actively waiting (`waiting_writers > 0`), incoming new readers MUST be forced to wait, allowing the writer to acquire the lock as soon as active readers drain.
2. Prove that a writer thread is never starved by continuous read traffic.

---

## 🔴 Advanced Level

### Exercise 3.1: The Fair Ticket Spinlock
Standard spinlocks using `atomic_flag.test_and_set()` suffer from **unfairness and starvation**: whichever CPU core happens to have the cache line hot when the lock is cleared wins the race.
1. Implement a **Ticket Lock** in C++:
   - Maintains two atomic counters: `std::atomic<int> next_ticket{0};` and `std::atomic<int> now_serving{0};`.
   - `lock()`: Fetches a unique ticket via `next_ticket.fetch_add(1)`, then spins until `now_serving.load() == my_ticket`.
   - `unlock()`: Increments `now_serving.fetch_add(1)`.
2. Explain how this algorithm mathematically guarantees strict **FIFO fairness** and bounded waiting ($O(N)$).
