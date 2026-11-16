# Solutions: Deadlocks & Resource Allocation

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Cycle Detection in RAGs
1. **Graph 1 contains a Deadlock**:
   - The cycle is: $P_1 \to R_2 \to P_2 \to R_3 \to P_3 \to R_1 \to P_1$.
   - Since every resource has only 1 instance, this cycle represents an unbreakable circular wait. The system is deadlocked.
2. **Graph 2 also contains a Deadlock**:
   - Trace edges: $P_2 \to R_3 \to P_3 \to R_2 \to P_2$.
   - There is a direct 2-node cycle between $P_2$, $R_3$, $P_3$, and $R_2$. Both $P_2$ and $P_3$ are deadlocked with each other. Consequently, $P_1$ (which waits for $R_2$) is also permanently blocked.

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Hand Trace of Banker's Algorithm

#### 1. Available Vector:
$$\text{Total Allocated} = [0+2+3+2, 1+0+0+1, 0+0+2+1] = [7, 2, 3]$$
$$\text{Available} = \text{Total} - \text{Total Allocated} = [10, 5, 7] - [7, 2, 3] = [3, 3, 4]$$

#### 2. Need Matrix ($\text{Max} - \text{Allocation}$):
- $P_0$: $[7-0, 5-1, 3-0] = [7, 4, 3]$
- $P_1$: $[3-2, 2-0, 2-0] = [1, 2, 2]$
- $P_2$: $[9-3, 0-0, 2-2] = [6, 0, 0]$
- $P_3$: $[4-2, 2-1, 2-1] = [2, 1, 1]$

#### 3. Safe State Verification:
- $\text{Work} = [3, 3, 4]$.
- Check $P_1$: $\text{Need}[1] = [1, 2, 2] \le [3, 3, 4]$ ✅ (Can complete!).
  - $\text{Work} = [3, 3, 4] + [2, 0, 0] = [5, 3, 4]$.
- Check $P_3$: $\text{Need}[3] = [2, 1, 1] \le [5, 3, 4]$ ✅ (Can complete!).
  - $\text{Work} = [5, 3, 4] + [2, 1, 1] = [7, 4, 5]$.
- Check $P_0$: $\text{Need}[0] = [7, 4, 3] \le [7, 4, 5]$ ✅ (Can complete!).
  - $\text{Work} = [7, 4, 5] + [0, 1, 0] = [7, 5, 5]$.
- Check $P_2$: $\text{Need}[2] = [6, 0, 0] \le [7, 5, 5]$ ✅ (Can complete!).
  - $\text{Work} = [7, 5, 5] + [3, 0, 2] = [10, 5, 7]$.
- **Safe Sequence exists**: $\langle P_1, P_3, P_0, P_2 \rangle$. The system is in a **SAFE STATE**.

#### 4. Handling $P_1$ Request $[1, 0, 2]$:
- Request $[1, 0, 2] \le \text{Need}[1] [1, 2, 2]$ ✅.
- Request $[1, 0, 2] \le \text{Available} [3, 3, 4]$ ✅.
- Tentative State:
  - $\text{Available} = [3, 3, 4] - [1, 0, 2] = [2, 3, 2]$.
  - $\text{Allocation}[1] = [3, 0, 2]$.
  - $\text{Need}[1] = [0, 2, 0]$.
- Check Safety: $\text{Need}[1] [0, 2, 0] \le \text{Work} [2, 3, 2]$ $\implies P_1$ finishes, releasing $[3, 0, 2]$. Work becomes $[5, 3, 4]$. Then $P_3$, $P_0$, $P_2$ complete.
- **Result: Safe sequence exists! The request is GRANTED immediately.**

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Instrumented Mutex Logic
```cpp
// Thread-Local stack of currently held locks
thread_local std::vector<int> t_held_locks;

void InstrumentedMutex::lock() {
    int requested_id = this->id_;
    // Check if adding edge (held -> requested) introduces a cycle in global lock graph
    for (int held_id : t_held_locks) {
        if (GlobalLockGraph::has_path(requested_id, held_id)) {
            throw std::runtime_error("DEADLOCK DETECTED: Inverted lock order between " 
                                     + std::to_string(held_id) + " and " + std::to_string(requested_id));
        }
        GlobalLockGraph::add_edge(held_id, requested_id);
    }
    raw_mutex_.lock();
    t_held_locks.push_back(requested_id);
}
```
This is the foundational pattern used by Clang's ThreadSanitizer (`-fsanitize=thread`) and database lock managers to detect deadlocks before deadlock-prone code reaches production.
