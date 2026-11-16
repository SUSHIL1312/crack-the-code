# Solutions: CPU Scheduling

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Gantt Chart & Metric Calculation

#### Workload:
- $P_1$: Arrival 0, Burst 8
- $P_2$: Arrival 1, Burst 4
- $P_3$: Arrival 2, Burst 9
- $P_4$: Arrival 3, Burst 5

---

### 1. FCFS Calculation:
- **Gantt Chart**:
  ```
  0        8        12        21        26
  ┌────────┬────────┬─────────┬─────────┐
  │   P1   │   P2   │   P3    │   P4    │
  └────────┴────────┴─────────┴─────────┘
  ```
- **Completion Times**: $P_1 = 8$, $P_2 = 12$, $P_3 = 21$, $P_4 = 26$.
- **Turnaround Time ($T_{completion} - T_{arrival}$)**:
  - $P_1 = 8 - 0 = 8$
  - $P_2 = 12 - 1 = 11$
  - $P_3 = 21 - 2 = 19$
  - $P_4 = 26 - 3 = 23$
  - **Avg Turnaround Time**: $(8 + 11 + 19 + 23) / 4 = 15.25\text{ ms}$.
- **Waiting Time ($\text{Turnaround} - T_{burst}$)**:
  - $P_1 = 8 - 8 = 0$
  - $P_2 = 11 - 4 = 7$
  - $P_3 = 19 - 9 = 10$
  - $P_4 = 23 - 5 = 18$
  - **Avg Waiting Time**: $(0 + 7 + 10 + 18) / 4 = 8.75\text{ ms}$.

---

### 2. SRTF (Preemptive SJF) Calculation:
- At $t=0$: $P_1$ starts (burst 8).
- At $t=1$: $P_2$ arrives with burst 4. $P_1$ remaining = 7. $P_2 (4) < P_1 (7)$, so **preempt $P_1$!** $P_2$ runs.
- At $t=2$: $P_3$ arrives (burst 9). $P_2$ remaining = 3. $P_2$ keeps running.
- At $t=3$: $P_4$ arrives (burst 5). $P_2$ remaining = 2. $P_2$ keeps running.
- At $t=5$: $P_2$ completes! Remaining: $P_4(5), P_1(7), P_3(9)$. Shortest is $P_4$. $P_4$ runs from 5 to 10.
- At $t=10$: $P_4$ completes! Remaining: $P_1(7), P_3(9)$. Shortest is $P_1$. $P_1$ runs from 10 to 17.
- At $t=17$: $P_1$ completes! $P_3$ runs from 17 to 26.
- **Gantt Chart**:
  ```
  0    1         5         10        17        26
  ┌────┬─────────┬─────────┬─────────┬─────────┐
  │ P1 │   P2    │   P4    │   P1    │   P3    │
  └────┴─────────┴─────────┴─────────┴─────────┘
  ```
- **Completion Times**: $P_2 = 5$, $P_4 = 10$, $P_1 = 17$, $P_3 = 26$.
- **Waiting Time**:
  - $P_1$: Waited $(10 - 1) = 9\text{ ms}$.
  - $P_2$: Waited $(1 - 1) = 0\text{ ms}$.
  - $P_3$: Waited $(17 - 2) = 15\text{ ms}$.
  - $P_4$: Waited $(5 - 3) = 2\text{ ms}$.
  - **Avg Waiting Time**: $(9 + 0 + 15 + 2) / 4 = 6.5\text{ ms}$ (vs 8.75 ms for FCFS!).

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Priority with Aging Algorithm
```cpp
void apply_aging(std::vector<Process>& procs, int current_time, int age_threshold_ms) {
    for (auto& p : procs) {
        if (p.remaining_time > 0 && p.arrival_time <= current_time) {
            // Process is waiting in Ready queue
            int time_spent_waiting = current_time - p.arrival_time - (p.burst_time - p.remaining_time);
            if (time_spent_waiting > 0 && time_spent_waiting % age_threshold_ms == 0) {
                p.priority++; // Increment priority score
            }
        }
    }
}
```
Aging dynamically breaks starvation by steadily lifting low-priority jobs into high-priority eligibility.

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: MLFQ Rules Verification
In MLFQ:
1. Short interactive tasks complete inside Queue 0 ($q=8\text{ ms}$) without being demoted, achieving near-zero response time.
2. Long computational tasks exhaust Queue 0 and Queue 1, descending to Queue 2 where they run with larger time slices, reducing context-switch thrashing.
3. The periodic 100 ms global boost ensures background compute jobs cannot be completely starved by continuous incoming interactive traffic.
