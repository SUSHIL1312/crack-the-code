# Module 00: Computer Fundamentals & Program Execution

<!-- Chapter Navigation Header -->
| ⬅️ Start of Track | 🏠 [Operating Systems Hub](../README.md) | [Module 01: Processes & Threads](../01-processes-and-threads/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Understanding how hardware, the operating system kernel, and user applications interact is the foundation of systems programming and high-performance engineering.

---

## 📑 Module Contents

1. [Theory & Core Architecture](theory.md)
   * The role of an OS: Resource Manager & Extended Machine.
   * Kernel space vs User space (CPU rings, protection boundaries).
   * Hardware hierarchy: Registers, Caches (L1/L2/L3), RAM, Storage.
   * System calls vs Standard library calls.
   * Interrupts, Traps, and Exceptions.
2. [Internal Working](internal-working.md)
   * Compilation pipeline: Preprocessor $\to$ Compiler $\to$ Assembler $\to$ Linker $\to$ Loader.
   * Static vs Dynamic linking mechanics.
   * Anatomy of an ELF binary (`.text`, `.rodata`, `.data`, `.bss`).
   * What happens before `main()` (`_start`, `__libc_start_main`, runtime initialization).
3. [Visual Explanations](visual-explanation.md)
   * CPU privilege level transitions (Ring 3 $\leftrightarrow$ Ring 0).
   * Memory segment layout (Stack, Heap, BSS, Data, Text).
   * System call trap dispatch sequence.
4. [Examples & Code](examples/)
   * `inspect_memory_layout.cpp`: Inspect addresses of stack, heap, data, bss, and code segments.
   * `syscall_vs_libcall.cpp`: Direct system call invocation via `syscall(SYS_write, ...)` vs `std::cout` / `printf`.
5. [Experiments](experiments/)
   * `inspect_elf_and_symbols.sh`: Hands-on exploration with `readelf`, `nm`, `size`, and `strace`.
6. [Exercises & Solutions](exercises/exercises.md)
   * Beginner, Intermediate, and Advanced challenges on segment layout and symbol resolution.
7. [Interview Questions & Deep Dives](interview-questions.md)
   * Categorized FAANG questions covering process launch, memory faults, and privilege boundaries.
8. [Common Mistakes](common-mistakes.md)
   * Traps regarding undefined behavior, stack overflow vs heap exhaustion, and syscall overhead.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| None (Track Start) | [Operating Systems Hub](../README.md) | **[Module 01: Processes & Threads](../01-processes-and-threads/README.md)** |
<!-- /Chapter Navigation Footer -->

