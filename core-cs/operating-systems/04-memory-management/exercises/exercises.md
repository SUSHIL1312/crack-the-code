# Exercises: Memory Management

---

## 🟢 Beginner Level

### Exercise 1.1: Struct Padding & Memory Alignment
Consider the following two C++ struct definitions on a 64-bit architecture:

```cpp
struct StructA {
    char a;     // 1 byte
    double b;   // 8 bytes
    int c;      // 4 bytes
    short d;    // 2 bytes
};

struct StructB {
    double b;   // 8 bytes
    int c;      // 4 bytes
    short d;    // 2 bytes
    char a;     // 1 byte
};
```

1. What is the value of `sizeof(StructA)`? Trace the exact byte offsets of each member and the inserted padding bytes.
2. What is the value of `sizeof(StructB)`?
3. What rule should engineers follow when ordering struct members to minimize wasted memory?

---

## 🟡 Intermediate Level

### Exercise 2.1: Extending `FreeListAllocator` with `realloc()`
1. In `free_list_allocator.cpp`, implement `void* reallocate(void* ptr, size_t new_size)`.
2. Optimize for in-place growth: If the physical block immediately following `ptr` is currently **free** and has sufficient capacity (`curr->size + next->size + sizeof(Header) >= new_size`), merge with the next block in-place without copying data!
3. If in-place growth is not possible, allocate a new block, copy data using `memcpy`, deallocate the old block, and return the new pointer.

---

## 🔴 Advanced Level

### Exercise 3.1: The Buddy Allocator Algorithm
The Linux kernel uses the **Binary Buddy Allocator** for managing physical page frames.
1. Given an arena of size $2^K = 1024\text{ KB}$, trace the allocation of:
   - Request A: 70 KB
   - Request B: 180 KB
   - Request C: 40 KB
2. How does the allocator compute the memory address of a block's "buddy" using bitwise XOR?
3. What are the key advantages of a Buddy Allocator over a standard Free-List allocator?
