# Modern C++: FAANG Interview Preparation Curriculum

A high-yield, code-first curriculum focused purely on the modern C++ (C++17/20/23) concepts tested in technical interviews at **Apple, Google, Bloomberg, Meta, NVIDIA, and Qualcomm**.

Zero academic fluff. Every module is structured around memory layouts, assembly implications, production code idioms, and live interview traps.

---

## 📚 Curriculum Structure

| Module | Core Concepts | High-Yield Interview Focus |
|---|---|---|
| **[01-memory-layout-and-pointers](01-memory-layout-and-pointers/)** | Struct alignment, padding, pointer arithmetic, `const` permutations. | Why `sizeof` differs from sum of members; cache line utilization. |
| **[02-raii-and-rule-of-five](02-raii-and-rule-of-five/)** | RAII, Rule of 5, copy-and-swap, `std::move` vs `std::forward`. | Preventing double free; resource management; move semantics. |
| **[03-smart-pointers-internals](03-smart-pointers-internals/)** | `unique_ptr`, `shared_ptr`, `weak_ptr`, control blocks. | `make_shared` vs `new`; breaking circular references; custom deleters. |
| **[04-polymorphism-and-vtables](04-polymorphism-and-vtables/)** | Virtual dispatch, vptr, vtable layout, multiple inheritance offsets. | Why virtual destructors are mandatory; vtable memory overhead. |
| **[05-templates-sfinae-and-concepts](05-templates-sfinae-and-concepts/)** | Templates, SFINAE (`std::enable_if_t`), C++20 Concepts (`requires`). | Compile-time constraints; type traits; readable compiler errors. |
| **[06-lambdas-and-functional-programming](06-lambdas-and-functional-programming/)** | Closures, captures, `mutable`, stateless decay, `std::function` SBO. | Capturing by reference in async tasks (UAF trap); SBO allocation. |
| **[07-concurrency-multithreading-and-atomics](07-concurrency-multithreading-and-atomics/)** | `std::jthread`, mutexes, `condition_variable`, atomics, memory orderings. | Acquire-Release vs Sequential Consistency; spurious wakeups. |
| **[08-modern-cpp-vocabulary-types](08-modern-cpp-vocabulary-types/)** | `std::optional`, `variant`, `string_view`, `span`, `expected`, `<=>`. | Zero-copy views without allocation; monadic error handling. |
| **[09-high-frequency-interview-bank](09-high-frequency-interview-bank/)** | Curated question bank with model answers and follow-ups. | High-frequency questions asked at Google, Apple, and Bloomberg. |

---

## 🛠️ Capstone Projects (`projects/`)

Battle-tested implementations of the 5 most common C++ coding interview project challenges:

1. **`01-custom-shared-ptr`**: Control block, atomic reference counting, custom deleter support, `make_shared` in-place allocation.
2. **`02-custom-vector`**: Dynamic reallocation, amortized $O(1)$ push_back, `std::move_if_noexcept` strong exception guarantee.
3. **`03-concurrent-bounded-queue`**: Thread-safe producer-consumer queue with condition variables and graceful shutdown.
4. **`04-concurrent-lru-cache`**: $O(1)$ lookup and eviction protected with reader-writer locking (`std::shared_mutex`).
5. **`05-thread-pool`**: Worker threads, task queue, `std::future` support via `std::packaged_task`.
