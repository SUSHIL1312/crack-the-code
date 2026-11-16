# Interview Questions: Context Switching

---

## Question 1: What is the exact difference between a Mode Switch and a Context Switch?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Systems Architecture & Kernel Interfaces
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Software Engineer, C++ Developer, SRE
* **Prerequisites**: Privilege rings, System calls, CPU registers
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute clear comparison with latency numbers.

### Model Answer
A **Mode Switch** is a transition in CPU execution privilege between **User Mode (Ring 3)** and **Kernel Mode (Ring 0)** (or vice-versa), while remaining within the context of the **same thread**.
- *Example*: Calling `read()` or `getpid()`.
- *Overhead*: Very low (~40–100 nanoseconds).
- *Action*: The CPU switches to the kernel stack and saves minimal registers (`pt_regs`). The thread identity, address space, and page tables (`CR3`) do NOT change.

A **Context Switch** is the transition of the CPU core from executing **one process or thread to a completely different process or thread**.
- *Example*: Time quantum expiration or a thread blocking on an empty I/O queue.
- *Overhead*: Moderate to high (~1–3 microseconds direct overhead, plus tens of microseconds in indirect cache misses).
- *Action*: The scheduler executes `pick_next_task()`, switches the kernel stack pointer (`RSP`), restores the new thread's CPU registers, and swaps page tables if switching between different processes.

---

## Question 2: What is a TLB Shootdown, and why is it expensive?

* **Difficulty**: Hard
* **Importance**: High
* **Category**: Multiprocessing & Virtual Memory
* **Classification**: Common industry interview theme
* **Role Relevance**: Kernel, Low-Latency & High-Performance Distributed Systems Engineers
* **Prerequisites**: Multicore architecture, Inter-Processor Interrupts (IPI), TLB
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3-minute architectural explanation of cross-core invalidation.

### Model Answer
A **TLB Shootdown** is the mechanism by which an operating system invalidates Translation Lookaside Buffer (TLB) entries across **multiple CPU cores** when a shared virtual memory mapping changes (e.g. after `munmap()`, page migration, or freeing shared memory).

### The Sequence:
1. Core 0 unmaps a virtual page from a multithreaded process.
2. Because threads of that process may be actively executing on Core 1, Core 2, and Core 3, their local hardware TLBs may still contain the stale virtual-to-physical address mapping.
3. Core 0 cannot directly write to another core's TLB. Instead, Core 0 sends an **Inter-Processor Interrupt (IPI)** to Cores 1, 2, and 3.
4. Each target core must immediately pause its current execution, switch to kernel mode, handle the interrupt, execute `invlpg` to flush the invalid TLB entry, and send an acknowledgment back to Core 0.
5. Core 0 stalls waiting for all remote cores to confirm the flush before freeing the physical RAM frame.

### Performance Impact:
IPIs interrupt running threads and incur pipeline stalls across the entire socket, often causing severe latency spikes in multi-socket, high-core NUMA systems.
