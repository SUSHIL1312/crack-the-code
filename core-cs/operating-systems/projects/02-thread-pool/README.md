# Project 02: Production-Grade C++20 Thread Pool

A modern, high-performance thread pool implementation in C++20 utilizing `std::jthread`, `std::condition_variable`, `std::packaged_task`, and `std::future`.

---

## Architectural Highlights
- **Task Submission API**: Flexible template method `submit(Callable&& f, Args&&... args)` returning `std::future<ReturnType>`.
- **Worker Thread Management**: Configurable worker thread count (defaults to `std::thread::hardware_concurrency()`).
- **Synchronized Task Queue**: Thread-safe task FIFO queue protected by `std::mutex` and notified via `std::condition_variable`.
- **Exception Propagation**: Any exceptions thrown inside a task are safely captured by `std::packaged_task` and rethrown upon calling `.get()` on the returned future.
- **Graceful Shutdown**: Destructor or explicit `shutdown()` drains pending tasks before safely joining all worker threads without thread leakage.

---

## Build and Run
```bash
make
./thread_pool_test
```
