# Interview Questions: Paging & Page Replacement

---

## Question 1: What is Belady's Anomaly, and why does it occur in FIFO but never in LRU?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Memory Management & Theoretical OS
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Software Engineer, All Core Engineering Candidates
* **Prerequisites**: Page replacement algorithms, Stack algorithms
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2–3 minute explanation using the Stack Property.

### Model Answer
**Belady's Anomaly** is the phenomenon where **increasing the number of physical page frames allocated to a process results in an INCREASE in the total number of page faults** for a given memory access pattern.

It occurs in algorithms like **First-In, First-Out (FIFO)** because FIFO evicts pages based purely on arrival order, without considering when the page was last used or when it will be needed next. Adding an extra frame alters the queue progression such that pages that are about to be accessed in the immediate future get evicted earlier.

### Why LRU is Immune (The Stack Property):
- **Least Recently Used (LRU)** belongs to a mathematical class known as **Stack Algorithms**.
- In a stack algorithm, for any reference string and any point in time, the set of pages resident in memory with $N$ frames is **guaranteed to be a strict subset** of the pages resident in memory with $N+1$ frames:
  $$M(N) \subset M(N+1)$$
- Therefore, any memory reference that is a cache hit with $N$ frames is mathematically guaranteed to also be a cache hit with $N+1$ frames. Thus, LRU can **never** suffer from Belady's Anomaly.

---

## Question 2: What is Thrashing, what causes it, and how does an OS detect and resolve it?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Performance Engineering & System Stability
* **Classification**: Officially documented interview topic
* **Role Relevance**: Systems, SRE, Performance Engineers
* **Prerequisites**: Virtual memory, Swap, Disk I/O, Page fault handler
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 3-minute explanation covering working sets and recovery mechanisms.

### Model Answer
**Thrashing** is a pathological operating system condition where the system spends significantly more time **paging data between RAM and swap storage** than executing user instructions.

### The Underlying Cause:
- Every active process has a **Working Set**—the collection of pages it actively references during its current execution phase.
- When the degree of multiprogramming is too high, the sum of all processes' working sets exceeds the available physical DRAM:
  $$\sum \text{WorkingSet}(P_i) > \text{Total Physical Memory}$$
- Process $P_1$ page-faults and evicts a page belonging to $P_2$'s working set. Then $P_2$ runs, page-faults immediately, and evicts a page belonging to $P_1$'s working set.
- All processes block on disk I/O. CPU utilization drops to near 0%, which foolishly prompts some naive schedulers to admit *more* processes into memory, compounding the disaster.

### Detection & Resolution:
1. **Working Set Tracking**: The kernel monitors the working set size of each process.
2. **Page Fault Frequency (PFF)**: If the system-wide page fault rate crosses an upper threshold $\text{PFF}_{\text{high}}$, thrashing is active.
3. **Resolution**:
   - The OS must reduce the degree of multiprogramming: suspend one or more low-priority processes and swap their entire memory images to disk.
   - The remaining active processes now have sufficient physical frames for their working sets, eliminating thrashing and restoring CPU utilization.
