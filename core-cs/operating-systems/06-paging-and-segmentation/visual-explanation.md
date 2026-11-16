# Visual Explanation: Page Replacement & The Clock Algorithm

## 1. The Clock (Second-Chance) Replacement Algorithm

The Clock algorithm arranges physical frames in a circular ring. A hand points to the next frame to examine.

```
                  Page 0 [A=1]
                     ▲
                 /       \
      Page 3 [A=0]       Page 1 [A=1]
                 \       /
                  Page 2 [A=0]
```

### Eviction Trajectory:
1. Clock hand points to Page 0:
   - Hardware Accessed bit $A == 1$.
   - Kernel clears bit: $A \gets 0$ (Page receives a second chance!).
   - Hand advances to Page 1.
2. Page 1 has $A == 1$:
   - Kernel clears bit: $A \gets 0$.
   - Hand advances to Page 2.
3. Page 2 has $A == 0$:
   - Hand selects **Page 2 for eviction!**
   - New page is loaded into Frame 2 with $A = 1$.
   - Hand advances to Page 3 for the next eviction cycle.

---

## 2. The Thrashing Curve

```
CPU
Utilization
   100% ▲
        │               Normal Paging
        │             .---.
        │           .'     `.
        │         .'         `.
        │       .'             `.
        │     .'                 \
        │   .'                    \  THRASHING COLLAPSE!
        │ .'                       \ (CPU spends 100% time waiting on disk)
        │/                          \
        └────────────────────────────\──────► Degree of Multiprogramming
                                  Threshold:
                          Sum(Working Sets) > RAM
```
