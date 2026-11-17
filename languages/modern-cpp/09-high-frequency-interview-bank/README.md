# Module 09: Modern C++ FAANG Technical Interview Question Bank

### Q1: What happens if an exception escapes from a destructor during stack unwinding?
**Answer**:
- If a destructor throws an exception while stack unwinding is already occurring for an earlier exception, the C++ runtime immediately invokes `std::terminate()`, which aborts the entire process without cleanup.
- This is because two simultaneous active exceptions cannot be propagated by the exception handling runtime.
- Consequently, destructors are implicitly `noexcept(true)` in Modern C++. All cleanup errors must be swallowed or logged inside the destructor.

---

### Q2: Why does `std::vector` fall back to copying elements if the move constructor is not marked `noexcept`?
**Answer**:
- To preserve the **Strong Exception Guarantee** ("commit or rollback").
- When a vector reallocates a larger memory buffer, it relocates elements using `std::move_if_noexcept`.
- If an element's move constructor can throw (`noexcept(false)`), moving element 2 could throw after elements 0 and 1 were already moved out (leaving the original vector half-emptied and unrecoverable).
- By falling back to deep copies, if a copy throws, the new buffer is discarded, leaving the original buffer 100% intact.

---

### Q3: Contrast `std::unique_ptr` and `std::shared_ptr` in terms of memory overhead and thread safety.
**Answer**:
- **`std::unique_ptr`**:
  - Size: 8 bytes (same as a raw pointer). Empty deleters take 0 bytes via Empty Base Optimization (EBO).
  - Overhead: Zero heap allocations beyond the managed object.
  - Thread safety: Non-atomic. Move-only.
- **`std::shared_ptr`**:
  - Size: 16 bytes (pointer to `T` + pointer to Control Block).
  - Control block contains atomic strong ref count, atomic weak ref count, deleter, and allocator.
  - Thread safety: The reference counts are modified using atomic operations (`fetch_add`, `fetch_sub`). However, concurrent reads/writes to the *pointed-to object* still require explicit application-level locking (mutex).

---

### Q4: Explain the difference between `const int*`, `int* const`, and `const int* const`.
**Answer**:
- `const int* p`: Pointer to a constant integer. The pointer `p` can point to another memory address, but `*p` cannot be modified.
- `int* const p`: Constant pointer to an integer. The pointer `p` cannot point to any other address, but `*p` can be modified.
- `const int* const p`: Constant pointer to a constant integer. Neither the address nor the value can be changed.

---

### Q5: What is the Curiously Recurring Template Pattern (CRTP), and when is it preferred over virtual functions?
**Answer**:
- CRTP is a static polymorphism idiom where a derived class inherits from a base class templated on the derived class itself:
  ```cpp
  template <typename Derived>
  class Base {
  public:
      void interface() { static_cast<Derived*>(this)->implementation(); }
  };
  class Impl : public Base<Impl> {
  public:
      void implementation() { ... }
  };
  ```
- **Why preferred**: Virtual functions require dynamic dispatch (vptr lookup + indirect call), which prevents compiler inlining and incurs branch misprediction penalties. CRTP resolves dispatch at compile-time with zero runtime overhead, enabling full inlining in high-frequency trading (HFT) and game loops.

---

### Q6: What is a data race under the C++11 memory model?
**Answer**:
- A program has a data race if two or more threads concurrently access the same memory location, at least one access is a write, and the accesses are not synchronized via atomic operations or mutex locks.
- In C++, a data race is **Undefined Behavior (UB)**. The compiler is permitted to generate arbitrary code, reorder instructions, or omit branches entirely.

---

### Q7: What is the difference between `std::memory_order_relaxed` and `std::memory_order_seq_cst`?
**Answer**:
- `relaxed`: Guarantees only that read and write operations on the specific atomic variable are atomic (no torn reads). It enforces **zero synchronization** or ordering constraints on surrounding memory accesses.
- `seq_cst` (Sequential Consistency): Default ordering. Enforces a single, globally agreed-upon total order of all atomic operations observed across all threads. It inserts hardware memory fences on weak memory architectures (ARM/POWER).

---

### Q8: What is Small Buffer Optimization (SBO) in `std::string` and `std::function`?
**Answer**:
- Allocating small objects on the heap is slow ($O(\text{malloc})$ overhead, memory fragmentation, cache misses).
- SBO embeds a small inline byte array (typically 15-22 bytes for `std::string`, 16-32 bytes for `std::function`) inside the object itself.
- If the data or closure fits within the internal buffer, zero heap allocations occur. Only when data exceeds the capacity does it allocate from the free store (heap).
