# Common Mistakes & Traps: CPU Scheduling

---

## 1. Confusing Response Time with Waiting Time

### The Misconception:
"If a process has low Waiting Time, it automatically provides great interactive responsiveness."

### The Reality:
- **Waiting Time** is the *cumulative sum* of all intervals the process spent in the Ready queue across its entire lifespan.
- **Response Time** is the duration between process arrival ($T_{arrival}$) and the *very first moment* it gets to execute on a CPU ($T_{start}$).
- In an interactive application (e.g., text editor or audio playback), users perceive **Response Time**. A program could have a long overall waiting time over 10 minutes, but if its initial response time is 1 ms, it feels instantaneous.

---

## 2. Believing SJF is Practical for General-Purpose Operating Systems

### The Misconception:
"SJF minimizes average waiting time, so production OSes should always use SJF."

### The Reality:
SJF requires the scheduler to know the **length of the next CPU burst in advance**, which is physically impossible for general-purpose user software. Schedulers can only approximate bursts using exponential smoothing historical averages:

$$\tau_{n+1} = \alpha t_n + (1 - \alpha)\tau_n$$

Even with estimation, SJF suffers from unbounded starvation for long jobs.

---

## 3. Assuming `nice -20` Magically Eliminates Latency Spikes

### The Pitfall:
Engineers sometimes run batch processing jobs or latency-sensitive microservices with `nice -20` (maximum non-realtime priority), expecting zero latency jitter.

### The Reality:
Under Linux CFS:
- A nice -20 process receives a **higher weight**, meaning its `vruntime` grows slower and it gets more relative CPU share over a scheduling epoch.
- **However, CFS is still a fair-share scheduler, NOT a hard real-time scheduler.** A nice -20 process can still be preempted by lower priority threads to enforce minimum granularity.
- If true deterministic, zero-jitter preemption is required, you must use POSIX real-time scheduling policies like `SCHED_FIFO` or `SCHED_RR` with `sched_setscheduler()`.
