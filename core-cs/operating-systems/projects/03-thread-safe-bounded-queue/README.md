# Project 03: Thread-Safe Bounded Blocking Queue

A production-quality blocking FIFO queue for Multi-Producer Multi-Consumer (MPMC) synchronization in Modern C++20.

---

## Architectural Highlights
- **Bounded Buffer Invariant**: Enforces a strict upper bound on memory consumption, preventing runaway producers from causing Out-Of-Memory (OOM) failures.
- **Two Condition Variables**:
  - `not_full_cv_`: Notifies blocked producers when space becomes available.
  - `not_empty_cv_`: Notifies blocked consumers when items are available.
- **Spurious Wakeup Protection**: All condition variable waits use predicate-based loops (`cv.wait(lock, [&] { return predicate; })`).
- **Timed Operations**: `try_push_for(item, duration)` and `try_pop_for(item, duration)` for robust timeout handling in distributed services.
- **Graceful Shutdown**: Unblocks all waiting threads immediately when closed, returning false or `std::nullopt`.

---

## Build and Run
```bash
make
./bounded_queue_test
```
