# Exercises: Paging & Page Replacement

---

## 🟢 Beginner Level

### Exercise 1.1: Hand Trace of FIFO and LRU
Given the memory reference string:
$$[7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2]$$
and a system with **3 physical page frames** (initially empty):

1. Trace the contents of the 3 frames step-by-step using **FIFO**. Count the total page faults.
2. Trace the contents of the 3 frames step-by-step using **LRU**. Count the total page faults.
3. Trace the contents of the 3 frames using **Optimal (MIN)**. Count the total page faults.

---

## 🟡 Intermediate Level

### Exercise 2.1: Enhanced Second-Chance Algorithm (Using the Dirty Bit)
In modern operating systems, evicting a **clean page** (Dirty bit $D=0$) is dramatically cheaper than evicting a **dirty page** ($D=1$), because a clean page can simply be discarded without writing to disk.
1. Extend the Clock algorithm to examine both the **Accessed Bit ($A$)** and **Dirty Bit ($D$)**, forming four preference classes:
   - **Class 0 $(A=0, D=0)$**: Neither recently used nor modified (Best candidate for eviction!).
   - **Class 1 $(A=0, D=1)$**: Not recently used, but modified (Requires writing to disk before eviction).
   - **Class 2 $(A=1, D=0)$**: Recently used, but clean (Likely to be used again soon).
   - **Class 3 $(A=1, D=1)$**: Recently used and modified (Worst candidate for eviction).
2. Write a function in C++ that scans the circular clock buffer in up to 4 passes to select a page from the lowest non-empty class.

---

## 🔴 Advanced Level

### Exercise 3.1: Thrashing Detection & Prevention Daemon
Write a simulation of a memory management monitor that tracks the system-wide page fault frequency (PFF):
1. If the average page fault rate per process exceeds an upper threshold $\text{PFF}_{\text{high}}$, the system is entering **Thrashing**.
2. When thrashing is detected, choose the lowest-priority process and **swap its entire working set out to disk** (suspending the process), freeing its physical frames for remaining processes.
3. Observe how CPU utilization and throughput for the remaining processes recover immediately once memory pressure is alleviated.
