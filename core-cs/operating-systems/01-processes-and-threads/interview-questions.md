# Interview Questions: Processes & Threads

---

## Question 1: Does a thread have its own stack? Can one thread access another thread's stack?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Memory Architecture & Concurrency
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Software Engineer, C++ Developer, Backend Infrastructure
* **Prerequisites**: Stack memory, Pointers, Virtual Memory
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2–3 minute explanation covering virtual address space sharing, pointer accessibility, and safety risks.

### Model Answer
**Yes, every thread has its own private execution stack.** The OS allocates a dedicated memory region for each thread's stack frames, local variables, and return addresses (e.g. 8 MB default on Linux for main thread, 512 KB–2 MB for worker threads).

**However, from a hardware MMU and virtual memory perspective, there is NO memory protection boundary between threads of the same process.** Because all threads share the exact same page table directory (`mm_struct`):
- If Thread A passes a pointer to a local stack variable to Thread B, Thread B **can read and write to Thread A's stack directly**.
- While syntactically and physically possible, doing this is exceptionally dangerous: if Thread A returns from its function before Thread B finishes, the pointer becomes a **dangling stack pointer**, leading to severe undefined behavior (stack corruption).

### Follow-up Questions
1. *Where does a thread's stack live in virtual memory?* (In the Memory Mapping / Anonymous mmap region between the heap and the main thread stack).
2. *What is a stack guard page?* (An unmapped, read/write-protected page placed at the end of each thread's stack. If a thread overflows its stack, writing to this guard page triggers an instant `SIGSEGV` instead of silently corrupting neighboring thread memory).

---

## Question 2: How does `fork()` behave in a multithreaded process?

* **Difficulty**: Hard
* **Importance**: High
* **Category**: Systems Programming & POSIX Internals
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Engineer, Core Backend Engineer
* **Prerequisites**: Multithreading, `fork()`, POSIX locks, Mutexes
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3-minute explanation highlighting single-thread inheritance, lock duplication, and async-signal safety.

### Model Answer
When `fork()` is called in a multithreaded process:
1. **Single-Thread Duplication**: The kernel duplicates the address space, but **only the single thread that invoked `fork()` exists in the child process**. All other threads simply vanish without running cleanup handlers or thread-specific destructors.
2. **Permanent Mutex Deadlock Hazard**:
   - If any other thread in the parent process held a mutex at the instant `fork()` ran, that mutex is copied into the child process in a **locked state**.
   - Because the owning thread does not exist in the child, that mutex will **never be unlocked**.
   - If the child attempts to acquire that mutex (directly or via standard library calls like `malloc` or `printf`), the child deadlocks permanently.
3. **Best Practices**:
   - In a multithreaded program, the child process after `fork()` should only execute **async-signal-safe functions** and immediately call an `exec` family function (`execve`) to wipe the memory image.
   - Alternatively, use `pthread_atfork()` to acquire all internal locks prior to the fork and release them in both parent and child.

---

## Question 3: Why are threads cheaper than processes to create and context switch?

* **Difficulty**: Easy
* **Importance**: Critical
* **Category**: OS Performance & Scheduling
* **Classification**: Officially documented interview topic
* **Role Relevance**: All Software Engineering Candidates
* **Prerequisites**: Virtual memory, TLB, Page tables, Cache locality
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute structured comparison covering creation overhead and context switch costs.

### Model Answer
Threads are cheaper than processes for two fundamental architectural reasons:

1. **Creation Overhead**:
   - Creating a **process** (`fork()`) requires allocating a new memory descriptor (`mm_struct`), creating and copying page tables (even with Copy-On-Write), duplicating file descriptor tables, and generating a new PID.
   - Creating a **thread** (`pthread_create()`) merely allocates a small Thread Control Block (TCB), a stack memory area, and points the new thread to the existing process's page tables and file descriptors. Benchmark measurements demonstrate thread creation is typically **15x to 20x faster**.
2. **Context Switching Overhead**:
   - **Thread switch (same process)**: The virtual memory address space does NOT change. CPU register `CR3` (page directory pointer) remains untouched. The CPU Translation Lookaside Buffer (TLB) remains valid, and CPU L1/L2 caches retain warm instructions and data.
   - **Process switch (different processes)**: The CPU must switch page directory base registers (`CR3` on x86, `TTBR0` on ARM), flushing or tagging the TLB. All virtual address lookups now incur cold cache and TLB misses, resulting in massive pipeline stall cycles.
