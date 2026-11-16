# Visual Explanation: Deadlocks & Lock Ordering Hierarchies

## 1. The Circular Wait Lock Ordering Bug

```
                 Thread 1                             Thread 2
        ┌─────────────────────────┐          ┌─────────────────────────┐
        │ Acquires Mutex A        │          │ Acquires Mutex B        │
        │                         │          │                         │
        │ Attempts to acquire     │          │ Attempts to acquire     │
        │ Mutex B                 │          │ Mutex A                 │
        │ (BLOCKED on Thread 2)   │          │ (BLOCKED on Thread 1)   │
        └────────────┬────────────┘          └────────────┬────────────┘
                     │                                    │
                     ▼                                    ▼
              Holds Mutex A ───(Waits for Mutex B)───► Held by Thread 2
                     ▲                                    │
                     │                                    │
                     └────────(Waits for Mutex A)─────────┘
                                 CIRCULAR WAIT
                                [DEADLOCK!]
```

---

## 2. Global Lock Ordering Defense (Hierarchical Locking)

Assign an integer hierarchy to all locks:
$$\text{Level}(A) = 1, \quad \text{Level}(B) = 2$$

**Rule**: A thread may only acquire a lock at Level $K$ if it holds no locks at Level $\ge K$.

```
Thread 1:
   Acquire Lock A (Level 1)
   Acquire Lock B (Level 2)  ✅ Allowed (Increasing order)

Thread 2:
   Acquire Lock A (Level 1)  (Blocks until Thread 1 finishes)
   Acquire Lock B (Level 2)  ✅ Circular wait is physically IMPOSSIBLE!
```
Under consistent lock ordering, cycles can never form in the Resource Allocation Graph.
