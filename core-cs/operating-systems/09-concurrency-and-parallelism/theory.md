# Theory: Concurrency, Parallelism & Multicore Hardware

## 1. Concurrency vs Parallelism

- **Concurrency**: The *composition* of independently executing processes or tasks. Dealing with a lot of things at once (e.g. single-core interleaved time-slicing).
- **Parallelism**: The *simultaneous execution* of multiple computational tasks at the exact same physical instant on separate hardware execution units (CPU cores or GPU ALUs).
- *Rob Pike's Dictum*: "Concurrency is about structure; Parallelism is about execution."

---

## 2. Amdahl's Law: The Ceiling of Scalability

When parallelizing software, performance gains are strictly bounded by the proportion of the program that is inherently **sequential (non-parallelizable)**.

Let $s$ be the fraction of the program that must run sequentially ($0 \le s \le 1$), and $N$ be the number of CPU cores. The maximum theoretical speedup $S(N)$ is:

$$S(N) = \frac{1}{s + \frac{1 - s}{N}}$$

### Key Consequence:
- As $N \to \infty$, the maximum achievable speedup is bounded by:
  $$\lim_{N \to \infty} S(N) = \frac{1}{s}$$
- **Example**: If just **5% ($s = 0.05$)** of your codebase is sequential (e.g. acquiring a global mutex or writing a sequential log), **no matter how many cores you throw at it (even 1,000,000 CPU cores), your speedup can NEVER exceed 20x!**

---

## 3. False Sharing & Hardware Cache Architecture

In modern multicore processors, each physical core has its own private, high-speed **L1 and L2 Caches**, while the **L3 Cache** is shared.

### What is False Sharing?
- CPUs load and invalidate memory in discrete **64-byte Cache Lines**.
- Suppose Thread A on Core 0 writes exclusively to integer variable `x`.
- Thread B on Core 1 writes exclusively to integer variable `y`.
- Logically, there is **no race condition**—the threads are accessing completely separate variables!
- **However**, if `x` and `y` happen to reside next to each other in memory within the **same 64-byte cache line**:
  1. Core 0 writes to `x`: The hardware cache controller marks the entire 64-byte line on Core 1 as **Invalid (`I`)**.
  2. Core 1 writes to `y`: Core 1 must stall its execution, fetch the updated cache line over the interconnect bus from Core 0, and invalidate Core 0's copy!
  3. This causes the cache line to rapidly "bounce" (ping-pong) back and forth across the CPU interconnect bus, degrading performance by **$5\times$ to $10\times$**!

---

## 4. Lock-Free vs Wait-Free vs Blocking

| Concurrency Level | Guarantee | Failure Mode Defense |
| :--- | :--- | :--- |
| **Blocking (Locks)** | Mutual exclusion via OS sleep. | If a thread holding a lock is delayed, suspended, or killed, **all other threads are blocked indefinitely**. |
| **Lock-Free** | At least **one thread** is guaranteed to make forward progress in a finite number of steps, system-wide. | Thread suspension cannot stall the entire system. Employs atomic loops (`compare_exchange_weak`). |
| **Wait-Free** | **Every thread** is guaranteed to make forward progress in a bounded number of its own steps, regardless of other threads. | Highest level of concurrency; extremely complex to implement. |
