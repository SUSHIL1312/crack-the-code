# Internal Working: Linux Two-List LRU, Kernel Watermarks, and Belady's Anomaly

## 1. How the Linux Kernel Reclaims Pages: The Two-List Strategy

True LRU is too expensive to maintain at hardware scale. Linux manages memory using **Two Linked Lists** for each memory zone:

```
        Page Referenced Again (Promoted)
        ┌──────────────────────────────────────────────┐
        │                                              │
        ▼                                              │
┌──────────────────────────────┐              ┌──────────────────────────────┐
│        ACTIVE LIST           │              │       INACTIVE LIST          │
│ (Hot pages, Reference Bit=1) │              │ (Cold pages, Candidates for  │
│                              │              │  Eviction)                   │
└──────────────┬───────────────┘              └──────────────┬───────────────┘
               │                                             │
               │ [Ages without reference: Demoted]           │ [Evicted / Paged to disk]
               └────────────────────────────────────────────►▼
                                                     Disk Swap / Discard
```

1. **Inactive List**: Holds pages that are candidates for eviction. Clean file pages here can be discarded immediately; dirty anonymous pages are written to swap.
2. **Active List**: Holds frequently referenced pages. The kernel will not evict pages from the active list.
3. **Promotion & Demotion**:
   - A page first faulted in is placed on the **Inactive List**.
   - If the page is referenced a second time while still on the Inactive List, it is promoted to the **Active List**.
   - A background scanner periodically scans the Active List; pages whose reference bits are un-accessed are cleared and demoted back to the Inactive List.

---

## 2. Memory Watermarks & `kswapd`

The kernel configures three watermarks per memory zone: `WMARK_MIN`, `WMARK_LOW`, and `WMARK_HIGH`.

```
Physical Memory Capacity
┌───────────────────────────────────────┐
│                                       │
│ Abundant Free Memory                  │
│                                       │
├───────────────────────────────────────┤ ◄── WMARK_HIGH (kswapd stops)
│                                       │
│ Moderate Free Memory                  │
│                                       │
├───────────────────────────────────────┤ ◄── WMARK_LOW (kswapd wakes up asynchronously)
│ Low Free Memory                       │
├───────────────────────────────────────┤ ◄── WMARK_MIN (Direct Reclaim! Allocating threads block)
│ Emergency Reserve (Atomic Allocations)│
└───────────────────────────────────────┘ 0 MB
```

- When free pages drop below `WMARK_LOW`, the asynchronous kernel daemon **`kswapd`** wakes up in the background and reclaims pages without stalling user applications.
- If memory consumption is too rapid and free memory hits `WMARK_MIN`, allocating threads **block synchronously in Direct Reclaim**, stalling application threads until enough memory is recovered.
- If memory drops to zero despite reclaim, the **OOM Killer** activates.

---

## 3. Belady's Anomaly: Mathematical Proof by Counterexample

**Belady's Anomaly** states that for certain page replacement algorithms (such as FIFO), **increasing the number of allocated page frames can result in an INCREASE in page faults**.

### The Classic Counterexample Reference String:
$$\text{Reference String} = [1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5]$$

#### Case A: 3 Page Frames (FIFO)
- Total Page Faults = **9 faults**.

#### Case B: 4 Page Frames (FIFO)
- Total Page Faults = **10 faults!**

### Why Does This Happen?
FIFO does not satisfy the **Stack Property**. In a stack algorithm (such as LRU or LFU), the set of pages present in an $m$-frame memory is guaranteed to be a strict subset of the pages present in an $(m+1)$-frame memory for any reference string:

$$M(m) \subset M(m+1)$$

Because FIFO discards pages based purely on arrival time regardless of usage frequency or recency, adding a 4th frame alters the eviction order such that critical upcoming pages are evicted earlier, counter-intuitively increasing page fault rates!
