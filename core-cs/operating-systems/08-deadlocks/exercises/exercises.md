# Exercises: Deadlocks & Resource Allocation

---

## 🟢 Beginner Level

### Exercise 1.1: Cycle Detection in Resource Allocation Graphs
Consider the following two Resource Allocation Graphs with Processes $P_1, P_2, P_3$ and single-instance Resources $R_1, R_2, R_3$:

**Graph 1:**
- $R_1 \to P_1$, $P_1 \to R_2$
- $R_2 \to P_2$, $P_2 \to R_3$
- $R_3 \to P_3$, $P_3 \to R_1$

**Graph 2:**
- $R_1 \to P_1$, $P_1 \to R_2$
- $R_2 \to P_2$, $P_2 \to R_3$
- $R_3 \to P_3$, $P_3 \to R_2$

1. Which graph contains a deadlock? Trace the exact cycle.
2. In Graph 2, does a deadlock exist? Explain why or why not.

---

## 🟡 Intermediate Level

### Exercise 2.1: Hand Trace of Banker's Algorithm
Consider a system with 4 processes ($P_0, P_1, P_2, P_3$) and 3 resource types ($A, B, C$) with total instance vector $[10, 5, 7]$:

- **Allocation**:
  - $P_0$: $[0, 1, 0]$
  - $P_1$: $[2, 0, 0]$
  - $P_2$: $[3, 0, 2]$
  - $P_3$: $[2, 1, 1]$
- **Max**:
  - $P_0$: $[7, 5, 3]$
  - $P_1$: $[3, 2, 2]$
  - $P_2$: $[9, 0, 2]$
  - $P_3$: $[4, 2, 2]$

1. Calculate the Available vector.
2. Calculate the Need matrix.
3. Determine if the system is currently in a Safe State. If so, write down the safe sequence.
4. If $P_1$ requests $[1, 0, 2]$, can the request be safely granted immediately?

---

## 🔴 Advanced Level

### Exercise 3.1: Runtime Lock Inversion Detector
Implement a custom `InstrumentedMutex` class in C++:
- Each mutex is assigned a unique string name or integer ID upon construction.
- When thread $T$ calls `lock()`, the class queries thread-local storage to see which mutexes $T$ *already holds*.
- It inserts directed edges $(\text{HeldMutex} \to \text{RequestedMutex})$ into a global adjacency list.
- Run Depth-First Search (DFS) or Tarjan's cycle detection. If a cycle is detected, **abort or throw an exception immediately**, alerting the developer to a latent deadlock bug before production deployment!
