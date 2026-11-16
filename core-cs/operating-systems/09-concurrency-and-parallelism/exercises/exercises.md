# Exercises: Concurrency & Multicore Parallelism

---

## 🟢 Beginner Level

### Exercise 1.1: Amdahl's Law Scalability Bounds
1. A video rendering application has a sequential startup/export phase that accounts for **4% ($s = 0.04$)** of its total execution time. The remaining 96% is perfectly parallelizable.
   - What is the theoretical speedup on an 8-core CPU?
   - What is the theoretical speedup on a 64-core server?
   - What is the maximum theoretical speedup even if given an infinite number of CPU cores ($N \to \infty$)?
2. If the team optimizes the sequential export phase to take only **1% ($s = 0.01$)**, what does the maximum theoretical speedup become?

---

## 🟡 Intermediate Level

### Exercise 2.1: Eliminating False Sharing in Thread Telemetry
Consider a high-throughput network proxy where 16 worker threads record metrics:
```cpp
struct WorkerMetrics {
    uint64_t packets_received[16]; // Thread i writes to packets_received[i]
};
```
1. Explain why this design suffers from severe False Sharing even though each thread writes to a unique array index.
2. Calculate how many cache lines are occupied by the array.
3. Refactor the struct in modern C++ to eliminate false sharing completely without changing the logic.

---

## 🔴 Advanced Level

### Exercise 3.1: The ABA Problem & Tagged Pointer
1. In `lock_free_stack.cpp`, explain why the naive implementation is theoretically vulnerable to the ABA problem if pointers are recycled by the allocator.
2. Design a `TaggedPointer<T>` struct that packs a 48-bit virtual address and a 16-bit monotonic counter into a 64-bit word using bitwise shifting and bitmasks.
3. Implement `compare_exchange_weak` on the `TaggedPointer` and explain how it prevents the ABA hazard.
