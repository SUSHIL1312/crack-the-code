# Solutions: Concurrency & Multicore Parallelism

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Amdahl's Law Calculations

Formula:
$$S(N) = \frac{1}{s + \frac{1 - s}{N}}$$

#### 1. With $s = 0.04$ (4% sequential):
- **For $N = 8$ cores**:
  $$S(8) = \frac{1}{0.04 + \frac{0.96}{8}} = \frac{1}{0.04 + 0.12} = \frac{1}{0.16} = \mathbf{6.25\times}$$
- **For $N = 64$ cores**:
  $$S(64) = \frac{1}{0.04 + \frac{0.96}{64}} = \frac{1}{0.04 + 0.015} = \frac{1}{0.055} \approx \mathbf{18.18\times}$$
- **For $N \to \infty$**:
  $$S(\infty) = \frac{1}{0.04} = \mathbf{25\times}$$

#### 2. With $s = 0.01$ (1% sequential):
- **For $N \to \infty$**:
  $$S(\infty) = \frac{1}{0.01} = \mathbf{100\times}$$
Reducing the sequential portion from 4% to 1% quadrupled the ultimate scalability limit from 25x to 100x!

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: False Sharing Telemetry Refactoring

1. **Why it suffers from False Sharing**:
   Each `uint64_t` is 8 bytes. The entire array of 16 elements occupies $16 \times 8 = 128$ bytes.
   On a standard CPU with 64-byte cache lines, the entire array fits into **just 2 cache lines**:
   - Cache Line 0 holds threads 0 to 7.
   - Cache Line 1 holds threads 8 to 15.
   Whenever Thread 0 increments its counter, Core 0 invalidates Cache Line 0 on Cores 1 through 7, causing massive bus bouncing.
2. **Refactored Code**:
   ```cpp
   #include <new>
   #include <atomic>

   struct alignas(64) PaddedWorkerMetric {
       std::atomic<uint64_t> packets_received{0};
       // Compiler automatically inserts 56 bytes of trailing padding
   };

   struct WorkerMetrics {
       PaddedWorkerMetric threads[16];
   };
   ```
   Now `threads[0]` through `threads[15]` each occupy their own isolated 64-byte cache line. All 16 threads can write concurrently at maximum memory bus efficiency!

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Tagged Pointer Implementation

```cpp
#include <cstdint>
#include <atomic>

template <typename T>
struct TaggedPointer {
    uintptr_t value; // Stores 48-bit address + 16-bit monotonic counter

    static constexpr uintptr_t ADDR_MASK = 0x0000FFFFFFFFFFFFULL;
    static constexpr uintptr_t TAG_MASK  = 0xFFFF000000000000ULL;
    static constexpr int TAG_SHIFT = 48;

    TaggedPointer(T* ptr = nullptr, uint16_t tag = 0) {
        value = (reinterpret_cast<uintptr_t>(ptr) & ADDR_MASK) | 
                (static_cast<uintptr_t>(tag) << TAG_SHIFT);
    }

    T* get_ptr() const {
        return reinterpret_cast<T*>(value & ADDR_MASK);
    }

    uint16_t get_tag() const {
        return static_cast<uint16_t>(value >> TAG_SHIFT);
    }

    TaggedPointer next_tag(T* new_ptr) const {
        return TaggedPointer(new_ptr, get_tag() + 1);
    }
};
```
By incrementing the 16-bit tag on every modification, even if the pointer address `A` is reused by the allocator, the tag will have changed ($0 \to 1 \to 2$). The CAS operation `compare_exchange_weak` checks the full 64-bit word (`value`), so it will safely detect that the node is a new incarnation and reject the ABA stale pointer.
