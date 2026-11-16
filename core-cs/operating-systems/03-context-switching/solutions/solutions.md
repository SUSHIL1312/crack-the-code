# Solutions: Context Switching

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: CPU Capacity Loss Calculation

#### Parameters:
- Quantum $q = 2\text{ ms} = 2,000\text{ }\mu\text{s}$.
- Direct switch overhead $T_{switch} = 2.5\text{ }\mu\text{s}$.
- Cache warming penalty $T_{cache} = 15\text{ }\mu\text{s}$.

1. **Direct Context Switching Loss**:
   $$\text{Loss}_{\text{direct}} = \frac{T_{switch}}{q + T_{switch}} = \frac{2.5}{2002.5} \approx 0.125\%$$
   (Negligible direct overhead under normal quantum).

2. **Total Effective Degradation (Direct + Cache)**:
   $$\text{Total penalty} = 2.5 + 15 = 17.5\text{ }\mu\text{s}$$
   $$\text{Degradation} = \frac{17.5}{2000 + 17.5} \approx 0.867\%$$

3. **Impact of Reducing Quantum to $0.2\text{ ms}$ ($200\text{ }\mu\text{s}$)**:
   $$\text{Total Overhead} = \frac{17.5}{200 + 17.5} = \frac{17.5}{217.5} \approx 8.05\%$$
   The server loses over **8% of its total computational capacity** strictly to context switching overhead and memory stall cycles!

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Thread vs Process Switch
- In thread context switching, both threads share the same virtual memory mapping (`CR3` register does not change).
- The Translation Lookaside Buffer (TLB) does not need to be flushed or tagged.
- The shared CPU caches (L1/L2) contain valid cache lines for both threads, resulting in significantly fewer cold cache misses compared to switching between two distinct heavy processes.

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Lazy vs Eager FPU Switching

1. **The Cost**: Saving 2 KB of AVX-512 register state on every context switch consumes hundreds of memory cycles, even if the scheduled thread never executes any floating-point or vector math!
2. **Lazy FPU Switching (Historical)**:
   - When switching tasks, the kernel simply set the **TS (Task Switched)** bit in CPU register `CR0` without touching the FPU registers.
   - If the new task only did integer arithmetic, the FPU state was never saved/restored (zero overhead).
   - If the task attempted to execute an FPU instruction, the CPU trapped with a `#NM (Device Not Available)` exception. The kernel's `#NM` handler would then finally save the old task's FPU state and load the new task's FPU state.
3. **Transition to Eager Switching (`XSAVEOPT`)**:
   - Modern software (glibc `memcpy`, `strlen`, cryptography) uses vector instructions constantly. Thus, the `#NM` trap was firing on almost every context switch, costing more than just saving the registers!
   - Modern CPUs introduced `XSAVEOPT` (Extended Save Optimized): hardware tracks which register state components have actually been modified and only writes dirty vector registers to memory, making eager switching hardware-accelerated and fast.
