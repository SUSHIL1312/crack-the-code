# Theory: Deadlocks, Coffman Conditions, and Strategies

## 1. What is a Deadlock?

A **Deadlock** is a situation where two or more threads are unable to proceed because each is waiting for the other to release a resource.

### The Four Coffman Conditions (1971)
For a deadlock to occur, all four of the following conditions must hold simultaneously:

1. **Mutual Exclusion**: At least one resource must be held in a non-shareable mode (only one process can hold it at a time).
2. **Hold and Wait**: A process must currently be holding at least one resource and actively waiting to acquire additional resources held by other processes.
3. **No Preemption**: Resources cannot be forcibly seized from a process; they can only be released voluntarily by the process holding them.
4. **Circular Wait**: A closed chain of processes $\{P_0, P_1, \dots, P_n\}$ exists such that $P_0$ is waiting for a resource held by $P_1$, $P_1$ is waiting for $P_2$, ..., and $P_n$ is waiting for $P_0$.

**If you break even ONE of these four conditions, a deadlock is mathematically impossible!**

---

## 2. Deadlock vs Livelock vs Starvation

| Condition | Threads Blocked? | CPU Utilization | Progression |
| :--- | :---: | :---: | :--- |
| **Deadlock** | ✅ Yes (Sleeping / Blocked in kernel) | 0% CPU | Permanent zero progress. Threads never resume without external intervention. |
| **Livelock** | ❌ No (Actively running) | 100% CPU | Zero progress! Threads continuously change internal state in response to each other (e.g. two people in a hallway repeatedly stepping left and right simultaneously). |
| **Starvation**| ✅ Yes | Variable | One or more low-priority threads are perpetually bypassed by higher-priority threads, but the system as a whole continues to make progress. |

---

## 3. The Four Paradigms for Handling Deadlocks

### 1. Deadlock Prevention (Design-Time Defense)
Invalidates at least one Coffman condition by design:
- *Break Mutual Exclusion*: Use lock-free data structures (atomics, read-copy-update).
- *Break Hold and Wait*: Force a process to request all resources at once, or release all held resources before requesting new ones.
- *Break No Preemption*: If a thread requesting lock B cannot get it, it must release lock A (`try_lock`).
- *Break Circular Wait (Most Common in Software)*: **Strict Global Lock Ordering**. Assign a total ordering to all locks ($L_1 < L_2 < \dots < L_n$). All threads must acquire locks in strictly increasing order.

### 2. Deadlock Avoidance (Runtime Dynamic Inspection)
- Uses knowledge of future maximum resource claims.
- Before granting a resource, the OS evaluates whether the resulting state is **Safe** or **Unsafe** (e.g. **Dijkstra's Banker's Algorithm**).
- If granting the request leads to an Unsafe state, the request is denied or delayed.

### 3. Deadlock Detection & Recovery
- Allows deadlocks to happen freely.
- Periodically runs a cycle-detection algorithm on the Resource Allocation Graph.
- When a cycle is detected:
  - *Recovery*: Abort one or more deadlocked processes, or rollback transactions to a checkpoint.

### 4. The Ostrich Algorithm
- Stick your head in the sand and pretend the problem doesn't exist.
- Used by general-purpose OSes (Linux, Windows, macOS) for user-space locks: detecting deadlocks on every lock acquisition is too expensive; the kernel lets deadlocked processes hang until killed by the user or watchdog timer.
