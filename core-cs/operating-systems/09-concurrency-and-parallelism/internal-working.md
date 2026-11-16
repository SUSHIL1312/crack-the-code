# Internal Working: MESI Cache Coherence & The Lock-Free ABA Problem

## 1. The MESI Cache Coherence Protocol

To ensure all CPU cores see a consistent view of shared physical memory, modern multicore hardware runs the **MESI Protocol** across the inter-core interconnect bus. Every 64-byte cache line in L1/L2 cache is tagged with one of four states:

```
┌───────────────┬────────────────────────────────────────────────────────────────────────┐
│ State         │ Meaning & Invariant                                                    │
├───────────────┼────────────────────────────────────────────────────────────────────────┤
│ **M (Modified)** │ Line is present ONLY in this core's cache and is **dirty** (modified). │
│               │ This core must write it back to memory before any other core reads it. │
├───────────────┼────────────────────────────────────────────────────────────────────────┤
│ **E (Exclusive)**│ Line is present ONLY in this core's cache and is **clean** (matches DRAM).│
├───────────────┼────────────────────────────────────────────────────────────────────────┤
│ **S (Shared)**   │ Line is present in this cache and potentially other cores' caches; clean.│
├───────────────┼────────────────────────────────────────────────────────────────────────┤
│ **I (Invalid)**  │ Line contains stale/invalid data. Any read requires an interconnect bus fetch.│
└───────────────┴────────────────────────────────────────────────────────────────────────┘
```

### The State Transition on a Write:
1. When Core 0 writes to a cache line in state `S` or `E`:
2. Core 0 broadcasts an **Invalidate Message** across the bus to all other cores.
3. Every other core containing that line updates its state to **Invalid (`I`)** and sends an Invalidate Acknowledgment back.
4. Core 0 transitions the line to **Modified (`M`)** and performs the write.

---

## 2. The Lock-Free ABA Problem

The **ABA Problem** is a insidious synchronization bug that occurs in lock-free algorithms utilizing **Compare-And-Swap (`CAS`)**:

```
Initial Stack: [ A ] ──► [ B ] ──► [ C ]
```

### The Step-by-Step Failure Sequence:
1. **Thread 1 prepares to Pop()**:
   - Reads `curr_top = A`.
   - Reads `next_node = A->next` (which is `B`).
   - Thread 1 is **preempted** right before executing `CAS(&top, A, B)`.
2. **Thread 2 executes multiple operations**:
   - Thread 2 pops `A`.
   - Thread 2 pops `B` and deletes it (`delete B`).
   - Thread 2 pushes a new node `D`.
   - Thread 2 allocates a new node and pushes it. The memory allocator recycles the previously freed address of `A`!
   - Stack is now: `[ A ] ──► [ D ] ──► [ C ]`.
3. **Thread 1 resumes**:
   - Thread 1 executes its delayed instruction: `CAS(&top, A, B)`.
   - The CPU checks: Is `top == A`? **YES! Address matches `A`!**
   - The CAS succeeds and assigns `top = B`.
4. **Catastrophe**:
   - Node `B` was already deleted and freed by Thread 2!
   - `top` now points to freed, unmapped, or recycled memory (`Use-After-Free`). Any subsequent read or write causes heap corruption or segfault!

```
Expected State: [ D ] ──► [ C ]
Actual Corrupted State: top ──► [ Deleted Garbage B ] (Corrupted!)
```

### The Solution: Tagged Pointers / Generation Counters
Pair every 64-bit pointer with a 64-bit monotonic generation counter (or pack both into a 128-bit structure updated via `CMPXCHG16B`):

$$\text{Pointer Value} = (\text{Address}, \text{Tag})$$

- When Thread 1 starts: `(A, 0)`.
- When Thread 2 re-pushes A: `(A, 2)`.
- Thread 1's `CAS` compares `(A, 0)` with `(A, 2)`. Because the tags differ ($0 \ne 2$), **the CAS correctly fails**, preventing corruption!
