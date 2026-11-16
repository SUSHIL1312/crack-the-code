# Interview Questions: Synchronization Primitives

---

## Question 1: What is the fundamental difference between a Mutex and a Binary Semaphore? Can Thread A lock and Thread B unlock?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Concurrency & Operating Systems
* **Classification**: Common industry interview theme
* **Role Relevance**: All Software Engineers
* **Prerequisites**: Mutexes, Semaphores, Critical sections
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2–3 minute explanation highlighting ownership and signaling.

### Model Answer
The core difference between a Mutex and a Binary Semaphore is **Ownership**:

1. **A Mutex is an Exclusion Lock WITH Ownership**:
   - The specific thread that invokes `mutex.lock()` **MUST** be the exact thread that invokes `mutex.unlock()`.
   - **Can Thread A lock and Thread B unlock? NO!** Attempting this invokes Undefined Behavior (UB) in POSIX and C++ standards (often causing an exception or crash).
   - Because a mutex tracks ownership, the OS kernel can implement **Priority Inheritance** to prevent priority inversion.
2. **A Semaphore is a Signaling Primitive WITHOUT Ownership**:
   - A semaphore is an integer counter with atomic increment (`post`/`release`) and decrement (`wait`/`acquire`) operations.
   - It has no concept of an "owner".
   - **Can Thread A wait and Thread B post? YES!** This is the intended design pattern for event notification (e.g., an interrupt handler thread signaling a worker thread).

---

## Question 2: What is Priority Inversion, and how does Priority Inheritance solve it?

* **Difficulty**: Hard
* **Importance**: Critical
* **Category**: Real-Time Systems & Kernel Scheduling
* **Classification**: Officially documented interview topic (The Mars Pathfinder Bug)
* **Role Relevance**: Systems Engineers, Embedded, Core Infrastructure
* **Prerequisites**: Preemptive scheduling, Priorities, Mutexes
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3-minute explanation with the low/medium/high priority thread triad.

### Model Answer
**Priority Inversion** is a scheduling hazard where a **high-priority thread is indirectly preempted and delayed by a low-priority thread**.

### The Scenario:
1. Low-priority thread $T_{\text{low}}$ acquires a shared Mutex $M$.
2. High-priority thread $T_{\text{high}}$ wakes up, preempts $T_{\text{low}}$, and attempts to acquire Mutex $M$. Since $M$ is locked, $T_{\text{high}}$ blocks.
3. Medium-priority thread $T_{\text{med}}$ (which does not need Mutex $M$) wakes up.
4. Because Priority($T_{\text{med}}$) > Priority($T_{\text{low}}$), the OS scheduler runs $T_{\text{med}}$, completely starving $T_{\text{low}}$.
5. **The Inversion**: Because $T_{\text{low}}$ cannot run, it can never release Mutex $M$. Thus, $T_{\text{med}}$ indirectly prevents the highest-priority thread $T_{\text{high}}$ from executing, inverting the system's priority guarantees! (Famous real-world example: The 1997 Mars Pathfinder spacecraft reset loop).

### The Solution: Priority Inheritance
When $T_{\text{high}}$ blocks waiting for Mutex $M$ held by $T_{\text{low}}$, the kernel temporarily **elevates the priority of $T_{\text{low}}$ to match $T_{\text{high}}$**.
- $T_{\text{low}}$ can no longer be preempted by $T_{\text{med}}$.
- $T_{\text{low}}$ quickly executes its critical section and releases Mutex $M$.
- As soon as the mutex is released, $T_{\text{low}}$ drops back to its original low priority, and $T_{\text{high}}$ immediately preempts and acquires the lock.

---

## Question 3: When should you choose a Spinlock over a Mutex?

* **Difficulty**: Medium
* **Importance**: High
* **Category**: Systems Performance & Optimization
* **Classification**: Common industry interview theme
* **Role Relevance**: Low-Latency C++, Linux Kernel Developers
* **Prerequisites**: Context switching overhead, Atomic test-and-set
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute quantitative trade-off analysis.

### Model Answer
A **Spinlock** should be chosen over a Mutex only when two strict criteria are met:
1. **The Critical Section is Ultra-Short (< 0.5 to 1 $\mu$s)**:
   - A mutex transition involves kernel futex sleep and wake: putting a thread to sleep and waking it up incurs a context switch cost of **~1 to 2 microseconds**.
   - If the thread only needs to modify a pointer or increment a counter (taking 10–20 nanoseconds), busy-waiting (spinning) on the CPU burns fewer clock cycles than putting the thread to sleep and waking it up.
2. **The System has Multiple Dedicated Physical Cores**:
   - On a single-core machine, a spinlock is an anti-pattern: the spinning thread prevents the lock-holding thread from ever running until its quantum expires!
3. **OS Kernel Interrupt Handlers**:
   - Interrupt service routines (ISRs) cannot sleep or call blocking mutexes; they MUST use spinlocks.
- **When NOT to use a Spinlock**:
  - Never use a spinlock if the critical section performs I/O, memory allocations, or waits for external events. Spinning under lock contention burns 100% CPU on all cores, generating heat, draining battery, and starving other tasks.
