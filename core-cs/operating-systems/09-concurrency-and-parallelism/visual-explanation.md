# Visual Explanation: False Sharing & The ABA Problem

## 1. False Sharing: Cache Line Ping-Ponging

```
Core 0                                                          Core 1
┌──────────────────────────────┐                               ┌──────────────────────────────┐
│ Thread A writes to var1      │                               │ Thread B writes to var2      │
├──────────────────────────────┤                               ├──────────────────────────────┤
│ L1 Cache:                    │                               │ L1 Cache:                    │
│ [ var1 (4B) | var2 (4B) ... ]│                               │ [ var1 (4B) | var2 (4B) ... ]│
│ State: MODIFIED              │───INVALIDATE BROADCAST───────►│ State: INVALID               │
│                              │                               │                              │
│                              │◄──INVALIDATE BROADCAST────────│ Thread B writes to var2:     │
│ State: INVALID               │                               │ State: MODIFIED              │
└──────────────────────────────┘                               └──────────────────────────────┘
                               ▲                               ▲
                               │                               │
                               └────────── MEMORY BUS ─────────┘
                                   (Massive Bus Contention!)
```

### The Fix: Cache Line Alignment Padding
Align each variable to its own 64-byte boundary using `alignas(64)`:

```
Core 0 Cache: [ var1 (4B) | 60 Bytes Padding ]  (Cache Line 0 - Core 0 Private)
Core 1 Cache: [ var2 (4B) | 60 Bytes Padding ]  (Cache Line 1 - Core 1 Private)
```
Now both cores execute simultaneously with zero cache invalidations and zero bus contention!

---

## 2. Amdahl's Law Speedup Curve

```
Speedup
  ▲
20│-------------------------------------------- Asymptotic Ceiling for s = 5% (Max 20x)
  │                                    .----''''
15│                                .--'
  │                            .--'
10│                        .--'
  │                    .--'
 5│               .---'
  │          .---'
  └─────────┴─────────┴─────────┴─────────┴────► Number of CPU Cores (N)
            4         8         16        64
```
Even with 10,000 cores, if 5% of the algorithm is sequential, speedup will never surpass 20x.
