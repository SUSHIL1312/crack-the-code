# Internal Working: Resource Allocation Graphs & The Banker's Algorithm

## 1. Resource Allocation Graphs (RAG)

A **Resource Allocation Graph (RAG)** is a directed graph $(V, E)$ where nodes $V$ are partitioned into two sets:
- **Processes ($P$)**: Represented as circles $\{P_1, P_2, \dots, P_n\}$.
- **Resource Types ($R$)**: Represented as boxes with dots (instances) $\{R_1, R_2, \dots, R_m\}$.

### Edges:
- **Request Edge ($P_i \to R_j$)**: Directed from process to resource. Indicates $P_i$ is currently blocked waiting for an instance of $R_j$.
- **Assignment Edge ($R_j \to P_i$)**: Directed from an instance of resource to process. Indicates $P_i$ currently holds that instance.

```
       [ Resource R1 ]
        ▲           │
        │           ▼
   (Process P1)  (Process P2)
        ▲           │
        │           ▼
       [ Resource R2 ]
```
- **Single-Unit Rule**: If every resource type has exactly **1 instance**, **a cycle in the RAG is both necessary and sufficient for a DEADLOCK**.
- **Multi-Unit Rule**: If resource types have multiple instances, a cycle indicates a *potential* deadlock, but not guaranteed (other threads can release instances and break the cycle).

---

## 2. The Banker's Algorithm (Dijkstra)

Named because a bank would never allocate its available cash in such a way that it could not satisfy the maximum loan requests of all its credit customers.

### Core Data Structures ($N$ Processes, $M$ Resources):
1. **`Available[M]`**: Number of available instances of each resource type.
2. **`Max[N][M]`**: Maximum demand matrix of each process.
3. **`Allocation[N][M]`**: Number of instances currently allocated to each process.
4. **`Need[N][M]`**: Remaining resource need:
   $$\text{Need}[i][j] = \text{Max}[i][j] - \text{Allocation}[i][j]$$

### The Safety Algorithm:
1. Let $\text{Work} = \text{Available}$, and $\text{Finish}[i] = \text{false}$ for all $i \in [0, N-1]$.
2. Find an index $i$ such that:
   - $\text{Finish}[i] == \text{false}$
   - $\text{Need}[i] \le \text{Work}$
   If no such $i$ exists, jump to Step 4.
3. If such an $i$ is found:
   - $\text{Work} = \text{Work} + \text{Allocation}[i]$ (Simulate process completing and releasing all its resources).
   - $\text{Finish}[i] = \text{true}$.
   - Go back to Step 2.
4. If $\text{Finish}[i] == \text{true}$ for ALL $i$, **the system is in a SAFE STATE**. There exists at least one execution sequence where every process completes without deadlocking! If not, the system is in an **UNSAFE STATE**.
