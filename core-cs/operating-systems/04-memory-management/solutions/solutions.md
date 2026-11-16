# Solutions: Memory Management

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Struct Padding & Memory Alignment

#### StructA Analysis:
- `char a` (1 byte): Offset `0`.
- Padding (7 bytes): Offset `1` to `7` (to align `double b` on an 8-byte boundary).
- `double b` (8 bytes): Offset `8` to `15`.
- `int c` (4 bytes): Offset `16` to `19`.
- `short d` (2 bytes): Offset `20` to `21`.
- Trailing Padding (2 bytes): Offset `22` to `23` (to round total struct size up to a multiple of its largest alignment, 8 bytes).
- **`sizeof(StructA) = 24 bytes`** (9 bytes wasted on padding!).

#### StructB Analysis:
- `double b` (8 bytes): Offset `0` to `7`.
- `int c` (4 bytes): Offset `8` to `11`.
- `short d` (2 bytes): Offset `12` to `13`.
- `char a` (1 byte): Offset `14`.
- Trailing Padding (1 byte): Offset `15` (to round total struct size up to a multiple of 8).
- **`sizeof(StructB) = 16 bytes`** (Only 1 byte of padding!).

#### Rule:
Always order members in **descending order of size/alignment** (8-byte types first, followed by 4-byte, 2-byte, then 1-byte). This packs members tightly with minimum compiler padding.

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: `realloc` with In-Place Growth
```cpp
void* reallocate(void* ptr, size_t new_size) {
    if (!ptr) return allocate(new_size);
    new_size = align8(new_size);

    BlockHeader* curr = reinterpret_cast<BlockHeader*>(
        reinterpret_cast<uint8_t*>(ptr) - sizeof(BlockHeader)
    );

    if (curr->size >= new_size) {
        return ptr; // Already large enough
    }

    // Check if next physical block is free and large enough to merge in-place
    if (curr->next && curr->next->is_free &&
        (curr->size + sizeof(BlockHeader) + curr->next->size >= new_size)) {
        
        size_t combined = curr->size + sizeof(BlockHeader) + curr->next->size;
        curr->next = curr->next->next;
        curr->size = combined;

        // Split remainder if large enough
        if (curr->size >= new_size + sizeof(BlockHeader) + 8) {
            split_block(curr, new_size);
        }
        return ptr; // In-place expansion successful! No memcpy needed.
    }

    // Fallback: Allocate new block, copy data, and deallocate old
    void* new_ptr = allocate(new_size);
    if (!new_ptr) return nullptr;
    std::memcpy(new_ptr, ptr, curr->size);
    deallocate(ptr);
    return new_ptr;
}
```

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: The Buddy Allocator Algorithm
1. **Tracing Requests**:
   - Total arena: 1024 KB.
   - Request A (70 KB): Needs smallest power of 2 $\ge 70\text{ KB}$, which is $128\text{ KB}$.
     - Split 1024 $\to$ (512, 512).
     - Split 512 $\to$ (256, 256).
     - Split 256 $\to$ (128, 128).
     - Allocate Block A in one 128 KB chunk.
   - Request B (180 KB): Smallest power of 2 is $256\text{ KB}$. Allocate from the free 256 KB block.
   - Request C (40 KB): Smallest power of 2 is $64\text{ KB}$. Split the remaining 128 KB buddy into two 64 KB blocks; allocate Block C.
2. **Buddy Address Computation via XOR**:
   $$\text{Buddy Address} = \text{Block Address} \oplus \text{Block Size}$$
   Because block sizes are exact powers of 2 ($2^k$), toggling the $k$-th bit immediately produces the base address of its adjacent twin (buddy).
3. **Advantage**: Fast $O(1)$ coalescing without scanning through linear linked lists.
