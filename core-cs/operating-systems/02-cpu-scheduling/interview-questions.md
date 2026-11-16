# Interview Questions: CPU Scheduling

---

## Question 1: How does the Linux Completely Fair Scheduler (CFS) work internally? What is `vruntime`?

* **Difficulty**: Hard
* **Importance**: Critical
* **Category**: Kernel Architecture & Scheduling
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Software Engineer, Linux Kernel Engineer, Backend Infrastructure
* **Prerequisites**: Red-Black trees, Virtual memory, Nice values
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3–5 minute detailed explanation of the ideal multitasking CPU model, `vruntime` scaling, and Red-Black tree dispatch.

### Model Answer
The **Completely Fair Scheduler (CFS)** is the default non-real-time CPU scheduler in Linux. Rather than using discrete time slices and priority queues, CFS models an **Ideal Multi-Tasking CPU** where $N$ runnable tasks run simultaneously, each receiving $1/N$ of the CPU bandwidth.

1. **Virtual Runtime (`vruntime`)**:
   - CFS tracks the normalized amount of CPU execution time a task has consumed.
   - When a task runs for physical wall-clock time $\Delta \text{exec}$, its `vruntime` increases according to:
     $$\Delta vruntime = \Delta \text{exec} \times \frac{\text{NICE\_0\_LOAD}}{\text{weight}}$$
   - A higher priority task (lower nice number) has a larger weight, so its `vruntime` advances **slower** per real microsecond. A lower priority task has a smaller weight, advancing its `vruntime` **faster**.
2. **Red-Black Tree Organization**:
   - Runnable tasks are stored in a self-balancing **Red-Black tree** (`cfs_rq`), sorted strictly by ascending `vruntime`.
   - The scheduler always picks the **leftmost node** (`rb_first`) to run next—the task that has consumed the least fair share of CPU time.
   - Picking the leftmost node is cached in $O(1)$ time. Re-inserting the task after its execution slice costs $O(\log N)$.
3. **Latency Target & Minimum Granularity**:
   - CFS uses a target scheduling period (e.g., 6–24 ms) during which all runnable tasks should run at least once.
   - To prevent thrashing when hundreds of tasks are ready, CFS enforces a `sysctl_sched_min_granularity` (e.g., 0.75 ms) below which a time slice is never reduced.

### Follow-up Questions
1. *What happens to `vruntime` when a task sleeps for 10 seconds and wakes up? Does it monopolize the CPU?* (No! To prevent starved wakeups from monopolizing the core, CFS clamps a waking task's `vruntime` to `min_vruntime - sysctl_sched_latency / 2`).
2. *Why did Linux 6.6 introduce EEVDF to replace standard CFS?* (To solve latency lag for interactive threads without relying on heuristic priority tweaks).

---

## Question 2: What is the Convoy Effect, and which algorithm causes it?

* **Difficulty**: Easy
* **Importance**: High
* **Category**: Algorithmic Performance
* **Classification**: Officially documented interview topic
* **Role Relevance**: All Software Engineers
* **Prerequisites**: FCFS, CPU vs I/O bursts
* **Estimated Preparation Time**: 10 minutes
* **Expected Answer Depth**: 2-minute concise answer with an intuitive analogy.

### Model Answer
The **Convoy Effect** is an operating system scheduling pathology associated with **First-Come, First-Served (FCFS)** non-preemptive scheduling.

It occurs when a single massive **CPU-bound process** gains the CPU first, forcing multiple short **I/O-bound processes** to sit idle in the Ready queue waiting behind it.
- **Analogy**: A giant slow-moving semi-truck on a single-lane highway holding up a line of 20 sports cars behind it.
- **Hardware Impact**: While the CPU-bound task is running, disk controllers and network interfaces sit completely idle. When the CPU-bound job finally issues an I/O request, all the waiting I/O-bound jobs quickly burst through the CPU and rush to I/O devices, leaving the CPU underutilized.
- **Resolution**: Use preemptive scheduling such as **Round Robin (RR)** or **Shortest Remaining Time First (SRTF)**, or Multilevel Feedback Queues that preempt CPU hogs.

---

## Question 3: What are the trade-offs of choosing a very small vs very large Time Quantum in Round Robin?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Systems Optimization
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems & Performance Engineers
* **Prerequisites**: Context switching, Hardware cache lines, CPU pipelines
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2–3 minute trade-off analysis spanning CPU cache effects and UI responsiveness.

### Model Answer
In Round Robin, the **Time Quantum ($q$)** governs preemption frequency:

1. **If Quantum is Too Large ($q \to \infty$)**:
   - The scheduler effectively degenerates into non-preemptive **FCFS**.
   - Average response time degrades severely. Interactive programs (typing in an editor, moving a mouse cursor) become sluggish and stutter.
2. **If Quantum is Too Small ($q \to 0$)**:
   - The system experiences severe **Context Switch Overhead**.
   - Saving and restoring registers, switching kernel stacks, and updating page tables costs ~1–2 microseconds per switch.
   - Crucially, frequent switching causes **Cache Thrashing**: each new process evicts the previous process's working set from the L1/L2 CPU caches. Memory access times jump from 1 ns (L1 cache) to 80 ns (DRAM).
3. **Engineering Sweet Spot**:
   - Schedulers aim for the time quantum to be roughly **100x to 1000x greater than the context switch overhead** (e.g. quantum of 10–20 ms vs switch overhead of 1–5 $\mu$s).
   - Rule of thumb: Approximately 80% of CPU bursts should be shorter than the chosen time quantum.
