# Modern C++ Capstone Projects

This directory contains the 5 most frequent live coding project challenges asked in senior/staff software engineer C++ interviews at Google, Apple, Meta, Bloomberg, and NVIDIA.

---

## 📂 Project Directory

| Project | Focus Area | What Interviewers Test |
|---|---|---|
| **[01-custom-shared-ptr](01-custom-shared-ptr/)** | Memory Management | Reference counting, control block heap layout, custom deleters, copy/move semantics. |
| **[02-custom-vector](02-custom-vector/)** | Container Internals | Dynamic reallocation, uninitialized memory placement new, `std::move_if_noexcept`. |
| **[03-concurrent-bounded-queue](03-concurrent-bounded-queue/)** | Concurrency | Condition variable wait loops, mutex locking, spurious wakeups, graceful shutdown. |
| **[04-concurrent-lru-cache](04-concurrent-lru-cache/)** | Data Structures & Locks | $O(1)$ Hash table + Doubly linked list, reader-writer lock (`std::shared_mutex`). |
| **[05-thread-pool](05-thread-pool/)** | Systems Architecture | Worker threads, work stealing/task queue, `std::future` and `std::packaged_task`. |
