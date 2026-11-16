# Visual Explanation: Memory Allocator Blocks & Cache Layouts

## 1. Anatomy of an Allocator Chunk (Boundary Tag Pattern)

To manage allocations and facilitate $O(1)$ merging of adjacent free blocks, allocators place invisible headers before the user payload:

```
┌───────────────────────────────┬───────────────────────────────────────────────┐
│ Chunk Header (16 Bytes)       │ User Payload (Returned to caller)             │
├───────────────┬───────────────┼───────────────────────────────────────────────┤
│ Prev Size     │ Size & Flags  │ Data returned by malloc()...                  │
│ (8 bytes)     │ (A | M | P)   │                                               │
│ [If prev free]│ [Total size]  │                                               │
└───────────────┴───────────────┴───────────────────────────────────────────────┘
                                ▲
                                │
                        ptr returned by malloc() points here!
```

- When the caller invokes `free(ptr)`, the allocator simply reads backwards: `Header* h = (Header*)((char*)ptr - sizeof(Header))`.

---

## 2. Array of Structures (AoS) vs Structure of Arrays (SoA)

Suppose we have 1,000,000 3D particles:

### A. Array of Structures (AoS) — Object-Oriented Layout
```cpp
struct Particle { float x, y, z; int id; char active; }; // 20 bytes (padded to 24)
std::vector<Particle> particles;
```

```
Memory Layout:
[x0, y0, z0, id0, act0] [x1, y1, z1, id1, act1] [x2, y2, z2, id2, act2] ...
|<──────────── Cache Line 0 (64 bytes) ───────────>|
```
*If a loop only updates `x`:* The CPU loads `y`, `z`, `id`, and `active` into L1 cache unnecessarily, wasting **75% of cache bandwidth** on unused fields!

---

### B. Structure of Arrays (SoA) — Data-Oriented Layout
```cpp
struct ParticleSystem {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
};
```

```
Memory Layout of x:
[x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15]
|<─────────────────────── Cache Line 0 (64 bytes) ─────────────────────>|
```
*When updating `x`:* Every single byte loaded into the L1 cache line is a relevant `float`. 100% cache efficiency + automatic SIMD vectorization!
