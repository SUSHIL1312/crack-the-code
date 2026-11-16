# Module 02: CPU Scheduling

CPU scheduling is the heart of multi-programmed operating systems. The scheduler decides which thread or process gets to execute on an available CPU core and for how long, optimizing for latency, throughput, responsiveness, and fairness.

---

## 📑 Module Contents

1. [Theory & Metrics](theory.md)
   * Preemptive vs Non-Preemptive scheduling.
   * CPU-bound vs I/O-bound workloads.
   * Core scheduling metrics: Turnaround time, Waiting time, Response time, Throughput.
   * Classic Algorithms: FCFS, SJF, SRTF, Round Robin (RR), Priority Scheduling with Aging.
   * Multilevel Feedback Queue (MLFQ) design principles.
2. [Internal Working & Linux Evolution](internal-working.md)
   * The evolution of Linux schedulers: $O(N) \to O(1) \to$ CFS (Completely Fair Scheduler) $\to$ EEVDF.
   * CFS internal mechanics: Red-Black trees, `vruntime` calculation, nice levels, latency target.
3. [Visual Explanations](visual-explanation.md)
   * Gantt charts illustrating FCFS Convoy Effect vs SJF.
   * Round Robin time quantum trade-off curves.
   * MLFQ priority decay and boost state machine.
4. [Examples & Code](examples/)
   * `scheduling_simulator.cpp`: Complete, robust C++ simulator implementing FCFS, SJF, SRTF, and Round Robin on identical workloads, computing and comparing precise metrics.
5. [Exercises & Solutions](exercises/exercises.md)
   * Calculation problems, time quantum tuning, and starvation analysis.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Key FAANG interview questions: CFS `vruntime`, Convoy effect, real-time priorities vs fairness.
7. [Common Mistakes](common-mistakes.md)
   * Selecting infinitesimal time quantums, ignoring I/O bursts, and confusing response time with waiting time.
