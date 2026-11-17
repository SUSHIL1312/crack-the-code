# Module 07: Concurrency, Multithreading & Atomics

<!-- Chapter Navigation Header -->
| ⬅️ [06: Lambdas & Functional Programming](../06-lambdas-and-functional-programming/README.md) | 🏠 [Modern C++ Hub](../README.md) | [08: Modern C++ Vocabulary Types](../08-modern-cpp-vocabulary-types/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. `std::jthread` (C++20) vs `std::thread` (C++14)

### 🔴 The C++14 Way (`std::thread` Pitfall):
```cpp
#include <thread>
#include <iostream>

void risky_task() {
    std::thread t([]() {
        std::cout << "Working...\n";
    });

    // FATAL BUG: If an exception is thrown here before t.join(),
    // t's destructor invokes std::terminate() and CRASHES the whole process!
    // In C++14, you had to manually wrap this in try-catch or write a ThreadGuard class.
    t.join();
}
```

### 🟢 The C++20 Way (`std::jthread` Auto-Joining RAII):
```cpp
#include <thread>
#include <iostream>

void safe_task() {
    // std::jthread auto-joins in its destructor on scope exit!
    std::jthread t([](std::stop_token st) {
        while (!st.stop_requested()) {
            std::cout << "Working safely...\n";
            break;
        }
    });
    // Zero risk of crash! If an exception throws, t.~jthread() safely joins.
}
```

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

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [06: Lambdas & Functional Programming](../06-lambdas-and-functional-programming/README.md) | [Modern C++ Hub](../README.md) | **[08: Modern C++ Vocabulary Types](../08-modern-cpp-vocabulary-types/README.md)** |
<!-- /Chapter Navigation Footer -->

