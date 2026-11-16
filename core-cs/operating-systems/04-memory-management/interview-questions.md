# Interview Questions: Memory Management

---

## Question 1: What happens under the hood when you call `malloc()` in C or `new` in C++?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Systems Programming & Memory Subsystems
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Engineer, C++ Developer, High-Performance Backend Engineer
* **Prerequisites**: Virtual memory, Heap segments, Glibc allocator
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 3-minute explanation covering runtime bins, kernel syscalls (`brk` vs `mmap`), and C++ constructor mechanics.

### Model Answer
When `new MyClass()` is called in C++:

1. **`operator new` Invocation**:
   - `new` evaluates `sizeof(MyClass)` and calls `operator new(size)`, which typically forwards to the C runtime `malloc(size)`.
2. **User-Space Allocator (e.g. glibc `ptmalloc`, jemalloc)**:
   - The allocator checks its thread-local cache or arena bins (Fastbins / Smallbins). If a cached free chunk matches the requested size, it is returned immediately in $O(1)$ time with **zero system calls**.
3. **Heap Expansion via System Calls**:
   - If no cached free chunk is available and the heap top is exhausted:
     - **For small allocations (< 128 KB)**: The allocator calls `sbrk()` or `brk()` to move the heap break upward in virtual address space.
     - **For large allocations ($\ge$ 128 KB)**: The allocator issues an `mmap(MAP_ANONYMOUS | MAP_PRIVATE)` system call to request dedicated virtual pages directly from the kernel.
4. **Lazy Physical Frame Allocation (Page Fault)**:
   - The kernel returns virtual memory addresses; **physical DRAM is not yet allocated**. Physical RAM is mapped lazily upon the first write via a hardware page fault.
5. **Constructor Execution**:
   - `operator new` returns the raw memory address (`void*`). The C++ compiler generates instructions to invoke `MyClass::MyClass()` at that address.

### Follow-up Questions
1. *What is the difference between `new` and `operator new`?* (`operator new` allocates raw untyped memory; the `new` expression calls `operator new` and then executes the type constructor).
2. *What happens if `malloc()` fails? What if `new` fails?* (`malloc()` returns `NULL` and sets `errno = ENOMEM`; `new` throws `std::bad_alloc` by default unless using `new (std::nothrow)`).

---

## Question 2: Why does sequential array access outperform random pointer access by up to 50x?

* **Difficulty**: Easy
* **Importance**: Critical
* **Category**: Hardware Architecture & Cache Locality
* **Classification**: Officially documented interview topic
* **Role Relevance**: All Software Engineers
* **Prerequisites**: CPU Caches, Cache lines, Spatial Locality
* **Estimated Preparation Time**: 10 minutes
* **Expected Answer Depth**: 2-minute clear explanation of hardware cache lines and hardware prefetchers.

### Model Answer
Sequential memory access (like iterating an array or `std::vector`) dramatically outperforms random access (like traversing a linked list or pointer-heavy binary tree) due to **CPU cache line design** and **hardware prefetchers**:

1. **Cache Line Transfers**:
   - CPUs never fetch single bytes or words from DRAM; they fetch chunks of **64 contiguous bytes** (a cache line).
   - In a sequential array of 4-byte integers, fetching the first integer loads 16 consecutive integers into the L1 cache. The next 15 reads are served in **1 clock cycle (~1 ns)** from L1 cache.
2. **Hardware Stream Prefetchers**:
   - Modern CPUs contain dedicated silicon that monitors memory access patterns. When the prefetcher detects sequential strides, it speculatively loads upcoming cache lines into L2/L1 cache *before* the CPU instructions even request them, hiding memory latency completely.
3. **Random Pointer Chasing Penalty**:
   - In a linked list or pointer tree, nodes are scattered across the heap. Each pointer dereference lands on a different, non-cached memory address.
   - The CPU stalls for **60 to 100 nanoseconds (~200 clock cycles)** waiting for data to travel across the bus from DRAM on *every single node access*.
