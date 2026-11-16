# Common Mistakes & Traps: Memory Management

---

## 1. Expecting Process RSS to Drop Immediately After `free()`

### The Misconception:
"I allocated 1 GB of memory using `malloc()`, processed data, and called `free()`. But when I run `top` or check Resident Set Size (RSS), the process still holds 1 GB of RAM! There must be a memory leak!"

### The Reality:
For heap allocations managed via `brk()`:
- `free()` returns the memory to the **glibc user-space allocator**, NOT to the operating system kernel.
- The allocator retains these chunks in its internal bins (Smallbins, Largebins) so subsequent allocations can be satisfied without expensive kernel mode transitions.
- Furthermore, the kernel can only shrink the heap via `brk()` if the **topmost chunk** of the heap is free. If even a tiny 16-byte object remains allocated at the very top of the heap, the entire heap below it cannot be released back to the OS!
- To force glibc to release unused heap memory back to the kernel, programs can invoke `malloc_trim(0)` on Linux.

---

## 2. Mismatching `new[]` and `delete`

### The Bug:
```cpp
int* array = new int[100];
delete array; // ❌ BUG: Should be delete[] array!
```

### Why It's Catastrophic:
- For primitive types (like `int`), some runtimes may mask the error, but it is **Undefined Behavior (UB)**.
- For objects with non-trivial destructors:
  - `new[]` allocates extra hidden prefix bytes (often 8 bytes) to store the **array length** so the runtime knows how many destructors to invoke during `delete[]`.
  - Calling scalar `delete` instead of `delete[]` misinterprets the pointer, skips destructors for elements 1 through 99, and feeds an offset address into the allocator, causing heap metadata corruption and instant segmentation faults.

---

## 3. Allocating Huge Buffers on the Thread Stack

### The Bug:
```cpp
void process_audio() {
    float buffer[2'000'000]; // ❌ Allocates 8 MB on the stack!
    // ...
}
```

### Why It Fails:
While the main thread typically has an 8 MB stack limit on Linux, worker threads created via `pthread_create()` or `std::thread` often have default stack sizes of only **512 KB to 2 MB** (and as low as 512 KB on macOS/Alpine musl).
Allocating large arrays on the stack instantly overflows past the stack guard page, triggering an immediate and unrecoverable `SIGSEGV`. Large buffers should always be allocated on the heap via `std::vector` or `std::make_unique`.
