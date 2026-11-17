# Module 07: Concurrency, Multithreading & Atomics

## 1. `std::jthread` vs `std::thread` (C++20)
`std::jthread` solves the biggest issue with C++11 `std::thread`:
- **Auto-joining on destruction**: Destructor automatically requests cancellation and calls `join()`, preventing `std::terminate()`.
- **Cooperative Cancellation**: Built-in support for `std::stop_token`.

---

## 2. Mutexes & RAII Lock Wrappers
- `std::unique_lock<std::mutex>`: Required for `condition_variable::wait()`. Supports manual unlocking and deferred locking.
- `std::scoped_lock<Mutexes...>`: Variadic RAII lock that prevents deadlocks when acquiring multiple mutexes simultaneously.
- **Condition Variable Spurious Wakeups**: Always enclose `wait()` in a predicate loop:
  ```cpp
  cv.wait(lock, [&]() { return ready; });
  ```

---

## 3. The C++11 Memory Model & Atomics
A **Data Race** occurs when two threads access the same memory location concurrently, at least one is a write, and there is no synchronization -> **Undefined Behavior**!

### The Six Memory Orderings:
1. `std::memory_order_relaxed`: Guarantees atomicity of the single variable only. No synchronization or barrier with surrounding instructions.
2. `std::memory_order_acquire`: Load operation. Ensures no subsequent memory reads/writes can be reordered before this load.
3. `std::memory_order_release`: Store operation. Ensures all previous memory reads/writes are committed and visible before this store.
4. `std::memory_order_seq_cst`: Sequential consistency (default). Globally agreed-upon total order across all threads.
