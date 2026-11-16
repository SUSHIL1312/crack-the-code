# Internal Working: The Linux Scheduler Evolution (O(N) to CFS & EEVDF)

## 1. The Architectural History of Linux Schedulers

| Era | Linux Version | Scheduler Algorithm | Primary Data Structure | Key Limitation |
| :--- | :--- | :--- | :--- | :--- |
| **Early Linux** | 2.4 | $O(N)$ Scheduler | Single global linked list | Scalability bottleneck; single spinlock contended by all CPU cores. |
| **SMP Scaling** | 2.6.0 – 2.6.22 | Ingo Molnar's $O(1)$ Scheduler | Per-CPU Active & Expired bitmapped arrays | Brittle heuristics attempting to detect interactive vs batch tasks. |
| **Fair Queuing** | 2.6.23 – 6.5 | Completely Fair Scheduler (CFS) | Red-Black Tree sorted by `vruntime` | Latency spikes for sleep/wake interactive threads competing with batch workloads. |
| **Modern Era** | 6.6+ | EEVDF (Earliest Eligible Virtual Deadline First) | Augmented Red-Black Tree tracking Virtual Deadlines | Replaces heuristics with mathematically rigorous lag and deadline balancing. |

---

## 2. Deep Dive: CFS (Completely Fair Scheduler) Mechanics

The core philosophy of CFS is to model an **Ideal Multi-Tasking CPU** on hardware: if there are $N$ runnable tasks, each task should run simultaneously at $1/N$ speed.

Since physical hardware can only run one thread per core at any given instant, CFS introduces the concept of **Virtual Runtime (`vruntime`)**:
- `vruntime` measures the amount of CPU execution time a task has consumed, scaled by its priority (nice level).
- **The Golden Invariant**: The task with the **smallest `vruntime`** has had the least fair share of CPU time, so it is chosen to run next.

### Mathematical Formulation
When a task runs for a physical duration $\Delta \text{exec\_time}$:

$$\Delta vruntime = \Delta \text{exec\_time} \times \frac{\text{NICE\_0\_LOAD}}{\text{task\_weight}}$$

- `NICE_0_LOAD` is calibrated to `1024`.
- A higher priority task (lower nice number, e.g. -5) has a larger `task_weight`, which makes its `vruntime` increase **slower**. Thus, it stays on the left of the Red-Black tree longer and receives more CPU time.
- A lower priority task (e.g. nice +5) has a smaller weight, making its `vruntime` advance rapidly.

```
CFS Red-Black Tree (cfs_rq):
                  [vruntime = 150ms]
                     /          \
    [vruntime = 100ms]          [vruntime = 220ms]
        /
[vruntime = 45ms] ◄── Leftmost Node (rb_first)
                      *Always chosen as NEXT to run!*
```

---

## 3. Nice Values to Weight Mapping

In Linux, nice values range from **-20 (highest priority)** to **+19 (lowest priority)**, with 0 as default.
Each step represents approximately a **10% geometric difference** in CPU allocation:

```c
// From kernel source: kernel/sched/core.c
const int sched_prio_to_weight[40] = {
    /* -20 */ 88761, 71755, 56483, 46273, 36291,
    /* -15 */ 29154, 23254, 18705, 14949, 11916,
    /* -10 */  9548,  7620,  6100,  4904,  3906,
    /*  -5 */  3121,  2501,  1991,  1586,  1277,
    /*   0 */  1024,   820,   655,   526,   423,
    /*   5 */   335,   272,   215,   172,   137,
    /*  10 */   110,    87,    70,    56,    45,
    /*  15 */    36,    29,    23,    18,    15,
};
```
Notice: $\frac{\text{weight}(nice=0)}{\text{weight}(nice=1)} = \frac{1024}{820} \approx 1.25$.
This mathematical elegance ensures that changing nice value by $+1$ always reduces CPU share proportionally regardless of the baseline.

---

## 4. Modern Evolution: EEVDF in Linux 6.6+

While CFS balanced throughput fairly, it suffered when interactive threads (like audio or UI event loops) woke up from sleep: they needed immediate low latency, not just long-term fair throughput.

Peter Zijlstra designed **EEVDF** based on Peter Hoon's 1995 paper:
1. **Eligibility**: A task is eligible to run only if its allocation lag is non-negative (it hasn't received more than its fair share).
2. **Virtual Deadline**: Each task requests a time slice $q$. Its deadline is calculated as:
   $$\text{Deadline} = \text{vruntime} + \frac{q}{\text{weight}}$$
3. The scheduler picks the task that is **both eligible and has the earliest virtual deadline**, guaranteeing bounded dispatch latency for latency-sensitive workloads.
