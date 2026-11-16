# Theory: CPU Scheduling Algorithms & Optimization Metrics

## 1. Why CPU Scheduling is Needed

In a modern multitasking system, multiple runnable processes compete for a finite number of physical CPU cores. The operating system's CPU scheduler determines:
1. Which process in the Ready queue runs next.
2. How long it may execute before being preempted.

### CPU Burst vs I/O Burst Cycle
Processes cycle between two phases:
- **CPU Burst**: Performing computation in registers and cache.
- **I/O Burst**: Waiting for disk reads, network packets, or user keystrokes.
- **CPU-bound processes**: Long CPU bursts, infrequent I/O (e.g. video rendering, scientific simulation, cryptographic hashing).
- **I/O-bound processes**: Short CPU bursts, frequent I/O (e.g. web servers, text editors, databases).

---

## 2. Preemptive vs Non-Preemptive Scheduling

- **Non-Preemptive (Cooperative)**: Once a process gets the CPU, it keeps running until it voluntarily yields control (via an I/O wait, sleep, or termination).
  - *Risk*: A rogue or infinite loop locks up the entire system.
- **Preemptive**: The OS kernel uses a hardware timer interrupt to regain control periodically (e.g. every 1–10 ms). The scheduler can forcibly suspend a running process and switch to another.

---

## 3. Core Evaluation Metrics

For a set of processes with arrival time $T_{arrival}$, start time $T_{start}$, burst time $T_{burst}$, and completion time $T_{completion}$:

$$\text{Turnaround Time} = T_{completion} - T_{arrival}$$
$$\text{Waiting Time} = \text{Turnaround Time} - T_{burst}$$
$$\text{Response Time} = T_{start} - T_{arrival}$$
$$\text{Throughput} = \frac{\text{Number of completed processes}}{\text{Total elapsed time}}$$

- **Turnaround Time**: Total lifespan of a process from arrival to completion.
- **Waiting Time**: Total time spent sitting in the Ready queue waiting for the CPU.
- **Response Time**: Time from submission until the first instruction executes (crucial for interactive UI responsiveness).

---

## 4. Classic Scheduling Algorithms

### 1. First-Come, First-Served (FCFS)
- Non-preemptive. Executes jobs in strict arrival order.
- **Fatal Flaw: The Convoy Effect**. If a massive CPU-bound job arrives first, short 1ms I/O-bound jobs queue behind it for seconds, drastically increasing average waiting time and stalling device utilization.

### 2. Shortest Job First (SJF) & Shortest Remaining Time First (SRTF)
- **SJF (Non-preemptive)**: Chooses the ready process with the smallest CPU burst time.
- **SRTF (Preemptive)**: If a new process arrives with a remaining burst shorter than the currently running job, the CPU is preempted.
- **Theoretical Guarantee**: SJF and SRTF are mathematically provable to yield the **minimum possible average waiting time**.
- **Practical Limitation**: Impossible for a general-purpose OS to know the future length of the next CPU burst (requires exponential smoothing estimation).
- **Flaw: Starvation**. A stream of short jobs can permanently starve long jobs.

### 3. Round Robin (RR)
- Designed specifically for interactive time-sharing.
- The scheduler runs a Ready queue as a FIFO ring, giving each process a fixed slice of time called a **Time Quantum** (or Time Slice, typically 10–100 ms).
- If the job hasn't finished when the quantum expires, the timer interrupt preempts it and appends it to the back of the Ready queue.
- **Quantum Trade-Off**:
  - Quantum too large ($\to \infty$): Degenerates into FCFS (poor response time).
  - Quantum too small ($\to 0$): System wastes all CPU cycles performing context switches and invalidating CPU caches!

### 4. Priority Scheduling & Starvation Defense (Aging)
- Each process is assigned an integer priority. Highest priority runs first.
- **Starvation Problem**: High-priority tasks keep entering, leaving low-priority tasks stranded indefinitely.
- **Solution — Aging**: The kernel gradually increments the priority of processes that sit waiting in the Ready queue for extended periods, guaranteeing every job eventually runs.

### 5. Multilevel Feedback Queue (MLFQ)
The gold standard for general-purpose desktop/server scheduling:
- Maintains multiple Ready queues with decreasing priority levels ($Q_0, Q_1, \dots, Q_k$).
- High-priority queues have short time quantums; lower queues have long quantums.
- **Rule 1**: If Priority(A) > Priority(B), A runs.
- **Rule 2**: If Priority(A) == Priority(B), Round Robin between them.
- **Rule 3**: New processes enter at top priority ($Q_0$).
- **Rule 4**: If a process uses up its entire time slice, its priority is reduced (moves down a queue).
- **Rule 5**: If a process voluntarily yields the CPU before its quantum expires (I/O burst), it stays at high priority.
- **Rule 6 (Anti-Starvation)**: Periodically boost all processes back to the top queue.
- *Result*: Automatically discovers I/O-bound interactive processes and keeps them responsive, while relegating batch compute jobs to lower queues!
