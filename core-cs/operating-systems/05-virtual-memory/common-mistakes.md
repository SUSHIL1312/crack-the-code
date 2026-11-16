# Common Mistakes & Traps: Virtual Memory

---

## 1. Confusing VSZ (Virtual Size) with RSS (Resident Set Size)

### The Misconception:
"When I run `ps aux`, my program shows `VSZ = 800 MB` and `RSS = 15 MB`. Does my application consume 800 MB of physical RAM?"

### The Reality:
- **VSZ (Virtual Memory Size)**: The total amount of virtual address space the process has *mapped* via memory regions (including shared libraries, reserved thread stack address spaces, and unused anonymous mappings). It costs **almost zero physical RAM**.
- **RSS (Resident Set Size)**: The actual amount of physical DRAM pages currently held in memory for this process.
- In the example above, the application is consuming only **15 MB of actual RAM**, not 800 MB!

---

## 2. Hardcoding 4096 (4 KB) as the Universal Page Size

### The Bug:
```cpp
// ANTI-PATTERN: Assuming every operating system and CPU has a 4 KB page size
constexpr size_t PAGE_SIZE = 4096; 
```

### Why It Fails:
While standard x86-64 uses 4 KB pages:
- **Apple Silicon (M1/M2/M3/M4)** natively uses **16 KB (16,384 bytes)** pages.
- Linux on ARM64 can be configured for 4 KB, 16 KB, or 64 KB pages.
- Enterprise database servers frequently configure **Huge Pages** (2 MB or 1 GB) using `madvise(MADV_HUGEPAGE)` to reduce TLB misses.
- **Best Practice**: Always query the runtime page size dynamically:
  ```cpp
  #include <unistd.h>
  long page_size = sysconf(_SC_PAGESIZE);
  ```

---

## 3. Ignoring Page Eviction in Low-Latency Trading Engines

### The Pitfall:
In ultra-low-latency financial trading or real-time audio systems, software engineers occasionally observe sudden 5-millisecond latency spikes in code paths that normally execute in 200 nanoseconds.

### The Cause:
Under memory pressure, the OS swapped out inactive code or data pages to disk. When a rare market event triggers that code path, the CPU encounters a **Major Page Fault**, stalling execution while fetching the page from SSD.

### The Fix:
Use `mlockall(MCL_CURRENT | MCL_FUTURE)` to permanently lock all current and future process pages into physical RAM, forbidding the OS from ever swapping them out.
