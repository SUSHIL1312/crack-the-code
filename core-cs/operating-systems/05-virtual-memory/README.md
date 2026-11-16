# Module 05: Virtual Memory & Address Translation

Virtual Memory is one of the greatest abstractions in computer systems. It provides every process with the illusion of an enormous, contiguous, and private address space while transparently mapping, protecting, and sharing physical DRAM underneath.

---

## 📑 Module Contents

1. [Theory & Architectural Foundations](theory.md)
   * The Motivation for Virtual Memory: Protection, Isolation, Resource Sharing, and Overcommit.
   * Virtual vs Physical Addresses.
   * The Memory Management Unit (MMU) & Address Translation.
   * Structure of a Page Table Entry (PTE): Present bit, R/W bit, User/Supervisor bit, Dirty bit, Accessed bit.
   * Multi-Level Page Tables: Why 4-level (PML4) and 5-level paging exist.
2. [Internal Working](internal-working.md)
   * Translation Lookaside Buffer (TLB) hits, misses, and Hardware Page Table Walks.
   * Anatomy of a Page Fault: Minor vs Major vs Invalid (`SIGSEGV`).
   * Demand Paging: Lazy physical frame allocation on first write.
   * Copy-on-Write (COW): How `fork()` creates instantaneous process duplicates.
3. [Visual Explanations](visual-explanation.md)
   * 4-Level Virtual Address Decomposition (x86-64 48-bit address translation diagram).
   * Page fault handling workflow inside the Linux kernel.
   * Copy-on-Write memory page splitting.
4. [Examples & Code](examples/)
   * `page_fault_benchmark.cpp`: Measures and proves Demand Paging by tracking Minor Page Faults (`getrusage`) when touching freshly allocated virtual pages.
   * `copy_on_write_demo.cpp`: Demonstrates Copy-on-Write page splitting after `fork()`, proving zero-overhead process duplication.
5. [Exercises & Solutions](exercises/exercises.md)
   * Multi-level page table size calculations, page table offset bitmasking, and COW cost analysis.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Critical FAANG questions: Page fault step-by-step trace, why 64-bit systems need multi-level tables, COW hazards.
7. [Common Mistakes](common-mistakes.md)
   * Equating Virtual Size (VSZ) with Resident Set Size (RSS), forgetting dirty page flushing.
