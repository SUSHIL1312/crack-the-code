# Project 04: Boundary-Tag Coalescing Memory Allocator

A custom implementation of dynamic memory management (`malloc` and `free`) using an explicit doubly-linked free list and Knuth's boundary-tag bidirectional coalescing.

---

## Architectural Highlights
- **Block Layout**:
  - **Header**: Block size (with lowest bit representing allocation status `0 = FREE, 1 = ALLOCATED`).
  - **Payload**: User memory space aligned to 8/16-byte boundaries.
  - **Footer**: Duplicate size tag placed at the end of each block, enabling $O(1)$ reverse-traversal coalescing with the preceding block in physical memory.
- **Search Policies**:
  - **First-Fit**: Fast allocation returning the first free block large enough.
  - **Best-Fit**: Minimizes internal fragmentation by finding the smallest block that satisfies the request.
- **Splitting**: When a free block is significantly larger than requested, it splits into an allocated chunk and a new smaller free chunk.
- **$O(1)$ Bidirectional Coalescing**:
  - Case 1: Neither preceding nor succeeding block is free.
  - Case 2: Only succeeding block is free $\to$ merge right.
  - Case 3: Only preceding block is free $\to$ merge left.
  - Case 4: Both preceding and succeeding blocks are free $\to$ merge both into one large contiguous block.

---

## Build and Run
```bash
make
./allocator_test
```
