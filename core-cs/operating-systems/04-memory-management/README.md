# Module 04: Memory Management

<!-- Chapter Navigation Header -->
| ⬅️ [Module 03: Context Switching](../03-context-switching/README.md) | 🏠 [Operating Systems Hub](../README.md) | [Module 05: Virtual Memory](../05-virtual-memory/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Memory management bridges the gap between raw physical RAM hardware and the structured memory allocations required by application software. Mastering allocation algorithms, fragmentation, and cache locality is fundamental to building high-performance systems.

---

## 📑 Module Contents

1. [Theory & Concepts](theory.md)
   * The Memory Hierarchy and Latency Gap.
   * Stack vs Heap vs Static vs Global Storage.
   * Internal Fragmentation vs External Fragmentation.
   * Classic Free Space Management: First Fit, Best Fit, Worst Fit, Segregated Free Lists.
   * Memory Alignment and Hardware Bus Fetching (`alignof`, `alignas`).
2. [Internal Working](internal-working.md)
   * How `malloc()` interacts with the OS: `brk()` / `sbrk()` vs `mmap(MAP_ANONYMOUS)`.
   * Glibc `ptmalloc3` architecture (Arenas, Fastbins, Smallbins, Largebins).
   * CPU Cache Lines (64-byte chunks) and Spatial/Temporal Locality.
3. [Visual Explanations](visual-explanation.md)
   * Memory fragmentation diagrams (Internal vs External).
   * Memory allocator boundary tag structure (Header, Payload, Alignment Padding).
   * Array of Structures (AoS) vs Structure of Arrays (SoA) cache lines.
4. [Examples & Code](examples/)
   * `free_list_allocator.cpp`: Complete, working custom Free-List Allocator implemented in C++ with block headers, splitting, and merging.
   * `cache_locality_aos_soa.cpp`: High-performance benchmark proving how cache-conscious memory layout (SoA vs AoS) yields massive speedups.
5. [Exercises & Solutions](exercises/exercises.md)
   * Calculating struct padding, fragmentation metrics, and allocator coalescing.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * What happens inside `malloc()`? Memory alignment traps, why sequential access beats random access.
7. [Common Mistakes](common-mistakes.md)
   * Use-after-free, double-free, forgetting memory alignment requirements, and false sharing.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 03: Context Switching](../03-context-switching/README.md) | [Operating Systems Hub](../README.md) | **[Module 05: Virtual Memory](../05-virtual-memory/README.md)** |
<!-- /Chapter Navigation Footer -->

