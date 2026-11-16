# Exercises: CPU Scheduling

---

## 🟢 Beginner Level

### Exercise 1.1: Gantt Chart & Metric Calculation
Given the following process workload:

| Process | Arrival Time | CPU Burst Time |
| :---: | :---: | :---: |
| $P_1$ | 0 ms | 8 ms |
| $P_2$ | 1 ms | 4 ms |
| $P_3$ | 2 ms | 9 ms |
| $P_4$ | 3 ms | 5 ms |

1. Draw the Gantt chart for **First-Come, First-Served (FCFS)**.
2. Draw the Gantt chart for **Shortest Remaining Time First (SRTF)**.
3. Calculate the Average Waiting Time and Average Turnaround Time for both algorithms.

---

## 🟡 Intermediate Level

### Exercise 2.1: Priority Scheduling with Aging
1. In standard static priority scheduling, low priority processes can suffer indefinite starvation.
2. Extend the scheduling simulator to implement **Priority Scheduling with Aging**:
   - For every 10 ms a process waits in the Ready queue without getting the CPU, increment its priority score by 1.
3. Demonstrate that a process with initial priority 1 (lowest) that would starve under static priority eventually preempts high-priority jobs.

---

## 🔴 Advanced Level

### Exercise 3.1: Multilevel Feedback Queue (MLFQ) Implementation
Implement a 3-level MLFQ simulator:
- **Queue 0**: Round Robin with Time Quantum $q = 8$ ms.
- **Queue 1**: Round Robin with Time Quantum $q = 16$ ms.
- **Queue 2**: First-Come First-Served (FCFS).
- Implement an Anti-Starvation periodic boost timer that resets all waiting processes to Queue 0 every 100 ms.
- Compare throughput and response times on mixed workloads containing both CPU-intensive rendering tasks and short interactive keyboard events.
