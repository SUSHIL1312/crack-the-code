# Module 03: Context Switching

<!-- Chapter Navigation Header -->
| ⬅️ [Module 02: CPU Scheduling](../02-cpu-scheduling/README.md) | 🏠 [Operating Systems Hub](../README.md) | [Module 04: Memory Management](../04-memory-management/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Context switching is the mechanism that allows a single CPU core to be shared among multiple threads and processes. While conceptually simple, its direct and indirect costs dictate system throughput, cache behavior, and latency limits.

---

## 📑 Module Contents

1. [Theory & Architecture](theory.md)
   * Definition of Context Switch.
   * CPU Hardware State: Program Counter (PC), Stack Pointer (SP), General-Purpose Registers, FPU/SIMD state.
   * Triggers: Preemptive Timer Interrupts, Blocking I/O, Yielding, Synchronization Waits.
   * Process Context Switch vs Thread Context Switch vs Mode Switch.
2. [Internal Working](internal-working.md)
   * The kernel `switch_to()` macro and CPU register state preservation.
   * Direct vs Indirect costs: Register push/pop vs TLB eviction and CPU cache pollution.
   * Address space switching (`CR3` on x86, `TTBR0` on ARM) and Process Context Identifiers (PCID / ASID).
3. [Visual Explanations](visual-explanation.md)
   * Register preservation timeline on the kernel stack.
   * Mode switch (Ring 3 $\to$ Ring 0) vs Full Context Switch ($P_1 \to P_2$).
4. [Examples & Code](examples/)
   * `benchmark_context_switch.cpp`: High-precision microbenchmark measuring exact context switch latency between processes using a bidirectional POSIX pipe ping-pong.
5. [Exercises & Solutions](exercises/exercises.md)
   * Quantitative problems calculating CPU capacity lost to context switching at high frequency.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * FAANG interview questions: Mode switch vs Context switch, TLB shootdowns, cache warm-up penalties.
7. [Common Mistakes](common-mistakes.md)
   * Confusing mode switches with context switches, over-threading workloads beyond hardware core counts.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 02: CPU Scheduling](../02-cpu-scheduling/README.md) | [Operating Systems Hub](../README.md) | **[Module 04: Memory Management](../04-memory-management/README.md)** |
<!-- /Chapter Navigation Footer -->

