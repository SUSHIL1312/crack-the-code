# Module 06: Paging, Segmentation & Page Replacement

When physical DRAM is saturated, the operating system must make critical decisions: which pages should remain in high-speed RAM, and which pages should be evicted to disk? This module explores paging vs segmentation, page replacement algorithms, Belady's anomaly, and system thrashing.

---

## 📑 Module Contents

1. [Theory & Concepts](theory.md)
   * Paging vs Segmentation: Fixed blocks vs Logical units.
   * Internal vs External fragmentation comparison.
   * Page Replacement Algorithms:
     - First-In, First-Out (FIFO) & Belady's Anomaly.
     - Least Recently Used (LRU) & Stack Algorithms.
     - Belady's Optimal Page Replacement (MIN).
     - Second-Chance / Clock Algorithm (Hardware Accessed Bit Approximation).
   * Working Sets & System Thrashing.
2. [Internal Working](internal-working.md)
   * How the Linux kernel approximates LRU: The Active and Inactive LRU Lists.
   * `kswapd` daemon, watermarks (`min`, `low`, `high`), and direct reclaim.
   * Belady's Anomaly mathematical proof: Why FIFO can suffer MORE page faults with MORE RAM!
3. [Visual Explanations](visual-explanation.md)
   * Clock algorithm circular pointer animation.
   * Thrashing curve: CPU utilization plummeting as degree of multiprogramming exceeds RAM capacity.
4. [Examples & Code](examples/)
   * `page_replacement_simulator.cpp`: Complete C++ simulator implementing FIFO, LRU, Optimal (MIN), and Clock algorithms, directly reproducing Belady's Anomaly on reference strings.
5. [Exercises & Solutions](exercises/exercises.md)
   * Hand-simulating page faults across algorithms, working set window sizing.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * What is Belady's Anomaly? How does Linux avoid thrashing? Clock vs LRU.
7. [Common Mistakes](common-mistakes.md)
   * Misunderstanding swap usage, ignoring the page cache when measuring memory.
