# Theory: Context Switching Mechanics & Costs

## 1. What is a Context Switch?

A **Context Switch** is the computational process of saving the execution state (context) of an active process or thread on a CPU core and loading the saved state of another ready process/thread so that execution can resume seamlessly.

### Execution Context Components
To resume an interrupted thread without side effects, the OS must preserve:
1. **Program Counter (PC / RIP / PC)**: The address of the next machine instruction.
2. **Stack Pointer (SP / RSP / SP)**: The current top of the execution stack.
3. **General-Purpose Registers**: `RAX`, `RBX`, `RCX`, `RDX`, `RSI`, `RDI`, `RBP`, `R8`–`R15`.
4. **Processor Status Flags**: Arithmetic condition codes (Zero flag, Carry flag, Sign flag).
5. **Floating-Point & Vector Registers**: AVX-512, SSE, or ARM NEON registers (saved lazily).
6. **Memory Mapping State**: Page table root address register (`CR3` on x86, `TTBR0` on ARM) for process switches.

---

## 2. When Does a Context Switch Occur?

1. **Preemptive Timer Interrupt**: The hardware APIC timer fires, triggering the kernel scheduler to enforce time quantum expiration.
2. **Voluntary Yield**: A thread calls `sched_yield()` or `sleep()`.
3. **Blocking I/O**: A thread issues a `read()` on an empty network socket or slow disk file, transitioning from **Running** to **Blocked**.
4. **Synchronization Contention**: A thread attempts to acquire an already-locked `std::mutex` or wait on a condition variable.

---

## 3. The Three Levels of Transitions

It is critical to distinguish between three frequently confused operations:

| Transition Type | Privilege Level Change? | Address Space (`CR3`) Change? | Schedulable Unit Change? | Typical Overhead |
| :--- | :---: | :---: | :---: | :--- |
| **Mode Switch** | ✅ Yes (Ring 3 $\to$ 0 $\to$ 3) | ❌ No | ❌ No (same thread continues) | ~40–100 ns |
| **Thread Context Switch** | ✅ Yes | ❌ No (shared address space) | ✅ Yes (Thread $A \to$ Thread $B$) | ~0.8–1.5 $\mu$s |
| **Process Context Switch**| ✅ Yes | ✅ Yes (Page tables swapped) | ✅ Yes (Process $A \to$ Process $B$)| ~1.5–3.5 $\mu$s + cache penalties |

---

## 4. Direct vs Indirect Overhead

The true performance penalty of a context switch is divided into two parts:

### A. Direct Overhead (Deterministic)
- Saving CPU registers to the thread's kernel stack.
- Running the scheduler algorithm (`pick_next_task`).
- Updating the CPU stack pointer to point to the new thread's kernel stack.
- Restoring the new thread's registers and executing `sysret` or `iret`.
- Direct cost: Typically **1 to 2 microseconds** (~2,000–6,000 CPU cycles).

### B. Indirect Overhead (The Silent Killer: Cache & TLB Pollution)
When a new process runs, the CPU's memory hierarchy is cold:
1. **L1 and L2 CPU Caches**: Contain the instructions and data lines of the *previous* process. The new process experiences immediate cache misses, forcing the CPU to fetch data across the memory bus from L3 cache or DRAM (~100x slower).
2. **Translation Lookaside Buffer (TLB)**: Caches virtual-to-physical address mappings. On a process switch without hardware tags, the TLB must be flushed. Every subsequent memory access incurs a hardware page-table walk.
3. **Branch Target Buffer (BTB)**: CPU branch predictors are mis-trained by the previous process, causing branch mispredictions and pipeline stalls.
- Indirect cost: Often **10 to 50 microseconds** of degraded application throughput while caches warm back up.
