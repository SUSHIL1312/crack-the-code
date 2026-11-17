# Programming Languages Curriculum

This section delivers an exhaustive, zero-to-advanced curriculum in two dominant engineering languages: **Modern C++** (for high-performance systems, low-latency applications, and infrastructure engineering) and **Python** (for scalable backend services, automation, runtime internals, and AI infrastructure).

---

## 📂 Subsystems

```
languages/
├── modern-cpp/      # High-Yield Modules + 5 Capstone Projects (C++17 / C++20 / C++23)
└── python/          # High-Yield Modules + 4 Capstone Projects (Python 3.12+ / CPython Internals)
```

---

## 1. [Modern C++](./modern-cpp/)
C++ is the cornerstone of high-performance systems and low-latency engineering:
* **Memory & Layout**: Struct padding, alignment, cache lines, raw pointers, and `const` permutations.
* **Resource Ownership**: RAII, Rule of Five, copy-and-swap, move semantics, and perfect forwarding.
* **Smart Pointers**: `unique_ptr` (zero-overhead EBO), `shared_ptr` (control blocks, atomic ref counts), `weak_ptr` (cycle breaking).
* **Polymorphism**: Dynamic dispatch, vptrs, vtable memory layout, multiple inheritance pointer adjustments, and virtual destructors.
* **Templates & Concepts**: SFINAE (`std::enable_if_t`), C++20 Concepts (`requires`), fold expressions.
* **Functional & Concurrency**: Closures, `mutable`, stateless decay, `std::jthread`, mutexes, `condition_variable`, atomics, and acquire-release memory orderings.
* **5 Capstone Projects**: Custom Shared Ptr, Custom Vector, Concurrent Bounded Queue, Concurrent LRU Cache, and Thread Pool.

---

## 2. [Python](./python/)
Taught with an uncompromising focus on CPython internals and production systems engineering:
* **Data Model & Dunders**: Pass-by-sharing, mutability pitfalls, hashability vs equality, and dunder methods catalog.
* **CPython Internals & GC**: `PyObject` headers, reference counting, and 3-generation cyclic garbage collector.
* **The GIL & Concurrency**: Global Interpreter Lock mechanics, CPU vs I/O workloads, and Python 3.13 free-threaded PEP 703 & JIT.
* **Asyncio & Structured Concurrency**: Single-threaded event loops, coroutines, and `asyncio.TaskGroup` child cancellation.
* **Decorators & Generators**: Closures, `functools.wraps`, lazy generator streaming pipelines, and descriptors.
* **4 Capstone Projects**: Async Rate Limiter, LRU Cache Decorator, Thread-Safe Connection Pool, and Execution Profiler Context Manager.
