# Solutions: Paging & Page Replacement

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Hand Trace Across 3 Frames

Reference String: `7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2` (3 frames)

#### 1. FIFO Trace:
- `7`: [7] (Fault 1)
- `0`: [7, 0] (Fault 2)
- `1`: [7, 0, 1] (Fault 3)
- `2`: Evict 7 $\to$ [2, 0, 1] (Fault 4)
- `0`: Hit
- `3`: Evict 0 $\to$ [2, 3, 1] (Fault 5)
- `0`: Evict 1 $\to$ [2, 3, 0] (Fault 6)
- `4`: Evict 2 $\to$ [4, 3, 0] (Fault 7)
- `2`: Evict 3 $\to$ [4, 2, 0] (Fault 8)
- `3`: Evict 0 $\to$ [4, 2, 3] (Fault 9)
- `0`: Evict 4 $\to$ [0, 2, 3] (Fault 10)
- `3`: Hit
- `2`: Hit
- **Total FIFO Faults = 10**.

#### 2. LRU Trace:
- `7`: [7] (Fault 1)
- `0`: [7, 0] (Fault 2)
- `1`: [7, 0, 1] (Fault 3)
- `2`: Evict 7 (least recently used) $\to$ [2, 0, 1] (Fault 4)
- `0`: Hit (LRU order becomes: 1, 2, 0)
- `3`: Evict 1 $\to$ [2, 0, 3] (Fault 5)
- `0`: Hit (LRU order becomes: 2, 3, 0)
- `4`: Evict 2 $\to$ [4, 3, 0] (Fault 6)
- `2`: Evict 3 $\to$ [4, 2, 0] (Fault 7)
- `3`: Evict 4 $\to$ [3, 2, 0] (Fault 8)
- `0`: Hit
- `3`: Hit
- `2`: Hit
- **Total LRU Faults = 8** (Outperforms FIFO by 20%!).

#### 3. Optimal (MIN) Trace:
- `7, 0, 1`: [7, 0, 1] (3 faults)
- `2`: Evict 7 (never used again) $\to$ [2, 0, 1] (Fault 4)
- `0`: Hit
- `3`: Evict 1 (never used again) $\to$ [2, 0, 3] (Fault 5)
- `0`: Hit
- `4`: Evict 2 (used furthest at pos 8 vs 3 at pos 9, 0 at pos 10) $\to$ [4, 0, 3] (Fault 6)
- `2`: Evict 4 $\to$ [2, 0, 3] (Fault 7)
- `3, 0, 3, 2`: All hits!
- **Total Optimal Faults = 7** (Provably minimum possible).

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Enhanced Second-Chance Implementation

```cpp
struct EnhancedClockFrame {
    int page;
    bool accessed;
    bool dirty;
};

int select_victim_enhanced_clock(std::vector<EnhancedClockFrame>& frames, int& hand) {
    int n = frames.size();

    // Pass 1: Look for Class 0 (A=0, D=0) without modifying bits
    for (int i = 0; i < n; ++i) {
        int idx = (hand + i) % n;
        if (!frames[idx].accessed && !frames[idx].dirty) {
            hand = (idx + 1) % n;
            return idx; // Best clean page found!
        }
    }

    // Pass 2: Look for Class 1 (A=0, D=1), clearing Accessed bit as we pass
    for (int i = 0; i < n; ++i) {
        int idx = (hand + i) % n;
        if (!frames[idx].accessed && frames[idx].dirty) {
            hand = (idx + 1) % n;
            return idx;
        }
        frames[idx].accessed = false; // Clear A bit
    }

    // Pass 3: Repeat Pass 1 (since all A bits are now cleared)
    for (int i = 0; i < n; ++i) {
        int idx = (hand + i) % n;
        if (!frames[idx].accessed && !frames[idx].dirty) {
            hand = (idx + 1) % n;
            return idx;
        }
    }

    // Pass 4: Select first available dirty page
    int victim = hand;
    hand = (hand + 1) % n;
    return victim;
}
```

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Thrashing Dynamics
When page fault frequency exceeds $\text{PFF}_{\text{high}}$, processes spend more time suspended waiting for disk queue DMA completions than executing CPU instructions.
By suspending a low-priority process, all of its physical frames are freed, expanding the allocation available to the remaining processes so their working sets fit in RAM. The average page fault frequency collapses back below $\text{PFF}_{\text{low}}$, and overall system throughput spikes back to normal.
