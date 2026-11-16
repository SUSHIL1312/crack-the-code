# Theory: Memory Management, Allocators, and Fragmentation

## 1. Stack vs Heap Memory

| Characteristic | Stack Memory | Heap Memory |
| :--- | :--- | :--- |
| **Allocation Cost** | $O(1)$ pointer bump (adjusts `RSP` register). Sub-nanosecond. | $O(1)$ to $O(N)$ searching free lists, lock contention. Tens to hundreds of nanoseconds. |
| **Deallocation Cost**| $O(1)$ automatic upon function return. | Explicit (`delete`, `free`), risk of leaks or fragmentation. |
| **Size Limit** | Fixed and small (typically 8 MB on Linux, 512KB–1MB on threads). | Dynamic, bounded by virtual address space and physical RAM/swap. |
| **Fragmentation** | Zero fragmentation (contiguous LIFO push/pop). | Vulnerable to internal and external fragmentation. |
| **Locality** | Excellent (hot cache lines in L1/L2). | Variable; scattered heap nodes cause cache misses. |

---

## 2. Memory Fragmentation: Internal vs External

```
A. INTERNAL FRAGMENTATION:
┌───────────────────────────────────────────────┐
│ Allocated Block: 64 Bytes                     │
│ ┌──────────────────────┬────────────────────┐ │
│ │ Payload (Requested): │ Unused Slack:      │ │
│ │ 37 Bytes             │ 27 Bytes (Wasted!) │ │
│ └──────────────────────┴────────────────────┘ │
└───────────────────────────────────────────────┘
(Memory allocated to a process is larger than requested, e.g. due to alignment or minimum chunk sizing).

B. EXTERNAL FRAGMENTATION:
┌──────────────┬──────────────┬──────────────┬──────────────┐
│ Used: 100 KB │ FREE: 40 KB  │ Used: 200 KB │ FREE: 50 KB  │
└──────────────┴──────────────┴──────────────┴──────────────┘
(Total free memory = 90 KB. However, a request for 70 KB FAILS because free space is split into non-contiguous blocks!).
```

- **Internal Fragmentation**: Occurs when an allocator rounds up an allocation request to satisfy alignment constraints (e.g. 8 or 16 bytes) or fixed bin sizes. The unused memory is *inside* the allocated block.
- **External Fragmentation**: Occurs when free memory is divided into small, non-contiguous chunks scattered across the heap. Although total free memory may exceed a requested size, no single continuous block is large enough.

---

## 3. Classic Free Space Management Strategies

When an application calls `malloc(size)`:
1. **First Fit**: Scans the free list from the beginning and returns the first block large enough.
   - *Pros*: Fast; keeps larger free blocks at the end of memory.
   - *Cons*: Tends to clutter the beginning of the free list with small fragments.
2. **Best Fit**: Scans the entire free list to find the block whose size is closest to the request.
   - *Pros*: Minimizes wasted space for the current allocation.
   - *Cons*: Slow ($O(N)$ search without segregated trees); creates tiny, unusable slivers of free memory.
3. **Worst Fit**: Allocates from the largest available block.
   - *Pros*: Leaves behind larger remaining blocks.
   - *Cons*: Destroys large contiguous blocks needed for big allocations.
4. **Segregated Free Lists (Buddy Allocator / Binning)**:
   - Modern allocators (jemalloc, TCMalloc, glibc ptmalloc) maintain separate linked lists for common fixed size classes (e.g. 16, 32, 64, 128, 256 bytes).
   - Allocating a known size class takes $O(1)$ with zero searching!

---

## 4. Memory Alignment & Hardware Mechanics

CPUs do not read memory from RAM byte-by-byte. The memory bus reads memory in **words** (4 bytes on 32-bit systems, 8 bytes on 64-bit systems) aligned to addresses that are multiples of word size.

- If an 8-byte `double` starts at an unaligned address (e.g. `0x1003` instead of `0x1000` or `0x1008`):
  - On x86-64: The CPU must issue **two separate memory bus reads**, shift the bytes, and merge them in a register, cutting memory throughput in half.
  - On ARM / MIPS: Unaligned memory accesses may trigger a hardware bus error (`SIGBUS`).
- **Struct Padding**: Compilers insert invisible padding bytes into structs so every member resides at an address that is a multiple of its natural alignment (`alignof(T)`).
