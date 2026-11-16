# Exercises: Context Switching

---

## 🟢 Beginner Level

### Exercise 1.1: CPU Capacity Loss Calculation
A database server runs 200 active worker threads on an 8-core CPU.
- The operating system scheduler uses a time quantum of $q = 2\text{ ms}$.
- Each context switch incurs an average direct overhead of $T_{switch} = 2.5\text{ }\mu\text{s}$.
- Due to cache eviction, each switch causes an additional indirect cache warming penalty of $T_{cache} = 15\text{ }\mu\text{s}$.

1. What percentage of total CPU time is wasted on direct context switching?
2. What percentage of total CPU capacity is degraded when including the cache warming penalty?
3. If an engineer reduces the time quantum to $0.2\text{ ms}$ (200 $\mu$s) to make threads "more responsive", what happens to the total available compute capacity?

---

## 🟡 Intermediate Level

### Exercise 2.1: Thread vs Process Context Switching
1. Adapt `benchmark_context_switch.cpp` to measure the context switch latency between **two threads** of the same process using `std::condition_variable` and a shared flag.
2. Compare the measured thread switch latency with the process switch latency.
3. Why is thread switching consistently faster than process switching?

---

## 🔴 Advanced Level

### Exercise 3.1: Hardware FPU & AVX Register Switching (`XSAVE`)
Modern x86-64 processors have 512-bit vector registers (`ZMM0`–`ZMM31` in AVX-512), amounting to over 2 KB of register state per thread.
1. Why does saving 2 KB of vector registers on every single context switch severely harm performance?
2. Explain how the Linux kernel historically used **Lazy FPU Switching** (via the `TS` task-switched bit in `CR0`).
3. Why did modern x86-64 CPUs transition to **Eager FPU Switching** using instructions like `XSAVEOPT` and `XRSTOR`?
