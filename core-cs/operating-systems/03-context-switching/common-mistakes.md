# Common Mistakes & Traps: Context Switching

---

## 1. Thread Oversubscription on CPU-Bound Workloads

### The Anti-Pattern:
```cpp
// ANTI-PATTERN: Spawning 500 threads for heavy mathematical image processing on an 8-core CPU
std::vector<std::thread> workers;
for (int i = 0; i < 500; ++i) {
    workers.emplace_back(heavy_computation_chunk, i);
}
```

### Why It Fails:
Physical hardware only has 8 execution pipelines.
- Spawning 500 compute-bound threads forces the OS scheduler into continuous, hyperactive preemption every few milliseconds.
- Every context switch evicts the CPU L1/L2 data caches. The execution pipelines spend more time waiting on memory fetches from DRAM and executing `switch_to()` than calculating actual numbers.
- **Rule of Thumb**: For pure CPU-bound tasks, thread count should never exceed `std::thread::hardware_concurrency()` (the number of logical hardware cores).

---

## 2. Using `sched_yield()` as a Synchronization Strategy

### The Anti-Pattern:
```cpp
// Spin waiting for a flag
while (!ready.load(std::memory_order_relaxed)) {
    sched_yield(); // ❌ Relinquishes CPU, but causes excessive context switches!
}
```

### Why It Fails:
`sched_yield()` forces an immediate context switch into the kernel. If no other thread is runnable, the kernel simply schedules the same thread right back, wasting time in kernel mode transitions. If another thread is runnable, the yielding thread is bumped to the back of the queue, introducing unpredictable latency.
- **Solution**: Use proper synchronization primitives (`std::condition_variable`, `std::atomic::wait`, or `futex`).
