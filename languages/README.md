# Programming Languages Curriculum

This section delivers an exhaustive, zero-to-advanced curriculum in two dominant engineering languages: **Modern C++** (for high-performance systems, low-latency applications, and infrastructure engineering) and **Python** (for scalable backend services, automation, runtime internals, and AI infrastructure).

---

## 📂 Subsystems

```
languages/
├── modern-cpp/      # 25 In-Depth Modules + 10 Capstone Projects (C++17 / C++20 / C++23)
└── python/          # 23 In-Depth Modules + 10 Capstone Projects (Python 3.12+ / CPython Internals)
```

---

## 1. [Modern C++](./modern-cpp/)
C++ is the cornerstone of high-performance engineering. This curriculum goes far beyond basic syntax:
* **Memory & Layout**: Stack vs heap, struct padding, memory alignment, strict aliasing, and vtable dispatch.
* **Resource Ownership**: RAII, Rule of Three/Five/Zero, copy semantics vs move semantics, perfect forwarding, and smart pointers.
* **Templates & Metaprogramming**: SFINAE, Concepts, fold expressions, type traits, and compile-time evaluation.
* **Concurrency & Systems**: Memory models, atomic operations, cache coherence, false sharing, and lock-free data structures.
* **10 Capstone Projects**: Custom STL Vector, Custom Smart Pointers, Memory Pool Allocator, Thread-Safe Queue, Thread Pool, Concurrent LRU Cache, Async Logger, TCP Chat Server, HTTP Server, and Parallel File Processor.

---

## 2. [Python](./python/)
Taught as an industrial software engineering language rather than a simple scripting tool:
* **Data Model & Special Methods**: Detailed catalog of dunder methods, MRO C3 linearization, object model, and descriptors.
* **CPython Internals**: Memory allocation (small object allocator), reference counting, generational GC, bytecode evaluation loop, and the evolving Global Interpreter Lock (including Python 3.13 free-threaded mode and JIT).
* **Typing & Metaprogramming**: Modern PEP 695 generics, Protocols, TypeVar, metaclasses, and dynamic attribute lookup.
* **Concurrency & Async**: Threading vs Multiprocessing vs Asyncio, TaskGroups, and structured concurrency.
* **10 Capstone Projects**: Custom Iterables Framework, Decorator-based Caching Engine, Dynamic Plugin Framework, Memory Profiler, Concurrent Scraper, Async HTTP Client, Multiprocessing Batch Executor, Async Task Scheduler, Thread-Safe In-Memory Cache, and Production-Grade CLI Tool.
