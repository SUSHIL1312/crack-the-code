# Theory: Paging, Segmentation, and Page Replacement Algorithms

## 1. Paging vs Segmentation

| Dimension | Paging | Segmentation |
| :--- | :--- | :--- |
| **Block Size** | Fixed hardware size (typically 4 KB, 2 MB, 1 GB). | Variable size matching logical programmer units (functions, arrays, stack). |
| **Visible to Programmer?**| Transparent; managed entirely by OS and hardware MMU. | Historically visible (segment registers: CS, DS, SS, ES). |
| **Fragmentation** | **Internal Fragmentation** only (last page of an allocation may be partially empty). Zero external fragmentation. | **External Fragmentation** (variable sized chunks leave scattered unusable gaps in memory). |
| **Modern Architecture** | Universally adopted across x86-64, ARM64, RISC-V. | Largely disabled on 64-bit systems (flat memory model, base = 0, limit ignored). |

---

## 2. Page Replacement Algorithms

When a page fault occurs and no physical memory frames are free, the kernel must choose a page to evict:

### 1. First-In, First-Out (FIFO)
- Replaces the page that was brought into memory earliest.
- Maintained as a simple queue.
- **Fatal Anomaly: Belady's Anomaly**. In FIFO, increasing the number of physical frames allocated to a process can actually **increase the number of page faults!**

### 2. Optimal Page Replacement (OPT / MIN / Belady's Algorithm)
- Replaces the page that will **not be used for the longest period of time in the future**.
- Mathematically provable to produce the minimum possible page fault rate for any given reference string.
- *Limitation*: Requires future prescience. It cannot be implemented in general-purpose OSes and serves as an unreachable theoretical benchmark.

### 3. Least Recently Used (LRU)
- Replaces the page that has not been accessed for the longest period of time in the past.
- Approximates the optimal algorithm based on the principle of **Temporal Locality**.
- Belongs to the class of **Stack Algorithms** (a set of pages in memory of size $N$ is always a strict subset of the pages in memory of size $N+1$). Therefore, **LRU can NEVER suffer from Belady's Anomaly**.
- *Hardware Challenge*: True LRU requires hardware to record a precise timestamp or update a doubly linked list on every single memory access, which is too expensive for high-speed hardware.

### 4. The Clock Algorithm (Second-Chance FIFO)
- Practical, low-overhead hardware approximation of LRU.
- Uses a single **Accessed / Reference Bit (A)** in the Page Table Entry, set automatically by CPU hardware on every read or write.
- Maintains a circular buffer (clock hand) pointing to pages:
  1. Inspect the page pointed to by the hand:
     - If $A == 1$: Give it a "second chance"—clear the bit ($A \gets 0$) and advance hand.
     - If $A == 0$: Select this page for eviction! Advance hand and stop.

---

## 3. Working Set Model & System Thrashing

- **Working Set**: The set of virtual pages a process has actively accessed within a recent time window $\Delta$.
- **Thrashing**: Occurs when the sum of working sets across all running processes exceeds the total capacity of physical RAM:

$$\sum_{i} \text{WorkingSet}(P_i) > \text{Total Physical RAM}$$

- When this threshold is crossed, every process page-faults continuously.
- The operating system spends **100% of its time waiting on disk I/O swapping pages in and out**, while CPU utilization plummets to near zero.
- **Recovery**: The OS must suspend/kill processes until the working set fits into physical RAM.
