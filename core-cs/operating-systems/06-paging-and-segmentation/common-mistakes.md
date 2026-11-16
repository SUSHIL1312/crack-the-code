# Common Mistakes & Traps: Paging & Segmentation

---

## 1. Disabling Swap Entirely on Linux Production Servers

### The Misconception:
"SSD/Disk swap is 1000x slower than RAM, so I will set `swapoff -a` to maximize performance!"

### The Reality:
Disabling swap entirely often harms production stability:
1. **Anonymous Pages Cannot Be Reclaimed**: During memory spikes, if swap is disabled, the kernel **cannot evict dirty anonymous pages**.
2. **Page Cache Depletion**: To free memory, the kernel is forced to evict clean executable code pages and shared libraries (`.text`). This causes heavy page-fault thrashing when the CPU re-executes code.
3. **Premature OOM Kills**: The system hits an abrupt memory cliff and the **OOM Killer** immediately terminates critical processes, where a tiny amount of swap could have absorbed the temporary spike.
- **Best Practice**: Keep a modest swap file (e.g. 2–4 GB) and set `vm.swappiness = 10` so Linux only uses swap under emergency memory pressure.

---

## 2. Assuming LRU Outperforms FIFO on All Access Patterns

### The Trap:
"LRU is always better than FIFO."

### The Counter-Scenario (Cyclic Looping):
Suppose you have **3 page frames** and iterate through an array spanning **4 pages** in a continuous loop:
$$\text{Reference String} = [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, \dots]$$

- **Under LRU**: Every access is a miss!
  - Access 1, 2, 3: Frames = [1, 2, 3].
  - Access 4: Evicts 1 $\to$ [2, 3, 4].
  - Access 1: Evicts 2 $\to$ [3, 4, 1].
  - **Result: 100% Page Fault Rate (0% Cache Hit Rate!)**.
- In cyclic loops with working set $> N$, MRU (Most Recently Used) or Random replacement actually outperforms LRU!
