# Interview Questions: Virtual Memory & Page Faults

---

## Question 1: What happens step-by-step during a Page Fault?

* **Difficulty**: Hard
* **Importance**: Critical
* **Category**: Hardware MMU & Kernel Internals
* **Classification**: Officially documented interview topic
* **Role Relevance**: Systems Software Engineer, Linux Kernel Engineer, Backend Infrastructure
* **Prerequisites**: Virtual memory, CPU exception vectors, MMU, Page tables
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3–5 minute detailed explanation from hardware trap to instruction replay.

### Model Answer
A **Page Fault** occurs when a CPU instruction attempts to access a virtual address that either has no physical translation present in DRAM (PTE Present bit = 0) or violates page permissions (e.g. writing to a read-only page).

### Step-by-Step Trajectory:
1. **Hardware MMU Exception**:
   - The CPU MMU tries to translate virtual address $V$. It reads the Page Table Entry (PTE) from cache/RAM and sees `Present = 0` (or `Write = 0` on a store instruction).
   - The MMU aborts the memory cycle and triggers **Interrupt Vector 14 (Page Fault)**.
   - The CPU saves the faulting address into register `CR2`, pushes the instruction pointer (`RIP`) and error code onto the Kernel Stack, and switches to **Ring 0 (Kernel Mode)**.
2. **Kernel VMA Lookup (`do_page_fault`)**:
   - The kernel's architecture-specific handler reads `CR2` and looks up the address in the process's list of **Virtual Memory Areas (VMAs)** (organized in a Red-Black tree inside `mm_struct`).
3. **Determining the Fault Type**:
   - **Case A: Invalid Access**: If the address falls outside all valid VMAs (e.g. `nullptr` or stack overflow beyond rlimit), or attempts a write on a read-only mapped segment (`.text`), the kernel sends a `SIGSEGV` signal to the process.
   - **Case B: Minor / Demand Paging Fault**: If the address is in a valid anonymous VMA (e.g. heap allocated with `mmap` or `malloc`), the kernel requests a fresh, zero-filled physical page frame from the Buddy Allocator, writes the physical frame number (PFN) into the PTE, sets `Present = 1`, and flushes the TLB.
   - **Case C: Major Page Fault (Disk / Swap)**: If the page is file-backed or was swapped out, the kernel puts the faulting thread into the **Blocked** state, issues an asynchronous I/O request to disk, and context-switches to another ready thread. Once disk I/O completes, the page is loaded into DRAM, the PTE updated, and the thread is unblocked.
   - **Case D: Copy-On-Write (COW)**: If the page is marked COW, the kernel allocates a new physical frame, copies the 4 KB content from the shared frame, updates the process's PTE to point to the new frame with Write permission, and decrements the shared frame's reference count.
4. **Instruction Replay**:
   - The kernel executes `iret` (interrupt return). The CPU returns to User Mode (Ring 3) and **re-executes the exact instruction that previously faulted**.
   - This time, the PTE is valid, the MMU completes the translation, and the program continues with zero awareness that a fault occurred!

---

## Question 2: What is the difference between Minor and Major Page Faults?

* **Difficulty**: Medium
* **Importance**: High
* **Category**: Systems Performance & Observability
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Engineers, Performance Engineers, SRE
* **Prerequisites**: Disk I/O latency, Memory management
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute clear comparison highlighting disk I/O latency.

### Model Answer
The difference between a **Minor Page Fault** and a **Major Page Fault** comes down to whether **Disk I/O is required**:

1. **Minor Page Fault (Soft Fault)**:
   - The required page **already exists in physical RAM**, but the process's page table simply does not have an active mapping to it.
   - *Causes*: Demand paging (first write to freshly allocated memory), Copy-on-Write frame duplication, or page cache hit (another process already loaded the file into the kernel page cache).
   - *Latency*: Very fast (**microseconds**). Executed entirely by CPU instructions and kernel memory pointers.
2. **Major Page Fault (Hard Fault)**:
   - The required page does **NOT exist in physical RAM** and must be fetched from persistent storage (SSD, HDD, or swap partition).
   - *Causes*: Cold read of a memory-mapped file (`mmap`), or reading a page that was evicted to swap due to high memory pressure.
   - *Latency*: Very slow (**tens of microseconds to milliseconds**). The thread must block, yield the CPU, wait for disk controller interrupts, and suffer a full context switch.
   - In production databases and low-latency engines, major page faults are a primary cause of tail latency (p99) spikes.
