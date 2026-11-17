# Module 02: RAII, Rule of Five & Move Semantics

<!-- Chapter Navigation Header -->
| ⬅️ [01: Memory Layout & Pointers](../01-memory-layout-and-pointers/README.md) | 🏠 [Modern C++ Hub](../README.md) | [03: Smart Pointers Internals](../03-smart-pointers-internals/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. What is RAII?
**RAII (Resource Acquisition Is Initialization)** binds resource lifetime to object lifetime:
- Resource (heap memory, socket, mutex lock, file descriptor) is acquired in the **constructor**.
- Resource is unconditionally released in the **destructor**.
- Stack unwinding during exceptions guarantees zero leaks.

---

## 2. Copy Constructor: Deep Copy vs. Shallow Copy (C++14 & Modern C++)

A **Copy Constructor** creates a new object as a copy of an existing object of the same class:
```cpp
ClassName(const ClassName& other);
```

### 🚨 Critical Interview Question: Why MUST the parameter be passed by reference (`const T&`)?
- **If passed by value (`ClassName(ClassName other)`)**:
  Passing an object by value requires creating a copy of the argument. Creating a copy calls the copy constructor. That copy constructor receives the argument by value, which calls the copy constructor again...
  $\to$ **Infinite compile-time recursion!** The C++ compiler forbids this with an error:
  `error: copy constructor must pass its first parameter by reference`.
- **Why `const`?**
  1. Guarantees the source object `other` will not be modified during copying.
  2. Allows copying from temporary rvalues (e.g. `Widget w = get_temp_widget();`), since `const T&` can bind to rvalues.

---

### ⚠️ Shallow Copy: The Compiler Default & The Double-Free Trap

If you don't define a copy constructor, the C++ compiler synthesizes a **Default Copy Constructor** that performs a member-by-member shallow copy:
- Primitive variables (`int`, `double`) are copied by value.
- **Pointers are copied by address value only!**

```
Shallow Copy Trap:
Original Object A:
[ data_ = 0x5500 ] ────┐
                        ▼
Copied Object B:       [ Heap Allocation: "FAANG" ] (Address: 0x5500)
[ data_ = 0x5500 ] ────┘
```

#### The Catastrophic Result:
1. **Shared State Corruption**: If Object B modifies `data_[0]`, it silently corrupts Object A's data!
2. **The Double-Free Crash**: When Object B goes out of scope, its destructor executes `delete[] data_` (freeing `0x5500`).
3. When Object A subsequently goes out of scope, its destructor attempts to execute `delete[] 0x5500` **again**!
4. **Crash**: The OS runtime aborts with `SIGABRT` (`free(): double free detected in tcache 2`). In production, this is also an exploitable security vulnerability (Heap Corruption).

---

### 🛡️ Deep Copy: Independent Resource Allocation

In a **Deep Copy**, the copy constructor allocates a **brand new memory buffer on the heap** and copies the actual contents from the source buffer:

```
Deep Copy:
Original Object A:
[ data_ = 0x5500 ] ───► [ Heap Buffer: "FAANG" ] (Address: 0x5500)

Copied Object B:
[ data_ = 0x7700 ] ───► [ Heap Buffer: "FAANG" ] (Address: 0x7700) (Independent Memory!)
```

---

### 💻 Production C++14 Implementation: Deep Copy & Assignment Operator

Here is the battle-tested, classic **C++14** implementation of an integer array managing raw heap memory:

```cpp
#include <iostream>
#include <algorithm> // for std::copy
#include <cstddef>   // for size_t

class DynamicArray {
private:
    int* data_;
    size_t size_;

public:
    // 1. Parameterized Constructor
    explicit DynamicArray(size_t size)
        : data_(size > 0 ? new int[size]() : nullptr), size_(size) {}

    // 2. Destructor (RAII)
    ~DynamicArray() {
        delete[] data_; // Safe even if data_ is nullptr
    }

    // 3. Deep Copy Constructor (C++14)
    DynamicArray(const DynamicArray& other)
        : data_(other.size_ > 0 ? new int[other.size_] : nullptr),
          size_(other.size_) {
        // Deep copy: allocate new memory and copy data bytes
        if (data_ && other.data_) {
            std::copy(other.data_, other.data_ + other.size_, data_);
        }
    }

    // 4. Deep Copy Assignment Operator (C++14)
    DynamicArray& operator=(const DynamicArray& other) {
        // STEP 1: Guard against Self-Assignment (e.g. arr = arr;)
        if (this == &other) {
            return *this;
        }

        // STEP 2: Allocate new memory FIRST (Exception Safety)
        int* new_data = other.size_ > 0 ? new int[other.size_] : nullptr;
        if (new_data && other.data_) {
            std::copy(other.data_, other.data_ + other.size_, new_data);
        }

        // STEP 3: Deallocate old resource
        delete[] data_;

        // STEP 4: Steal new resource pointer
        data_ = new_data;
        size_ = other.size_;

        return *this; // Return reference for chaining (a = b = c)
    }

    // 5. Move Constructor (C++14 / C++11)
    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr; // Leave source in valid destructible state
        other.size_ = 0;
    }

    // 6. Move Assignment Operator (C++14 / C++11)
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;          // Free existing buffer
            data_ = other.data_;     // Steal guts
            size_ = other.size_;
            other.data_ = nullptr;   // Reset source
            other.size_ = 0;
        }
        return *this;
    }

    size_t size() const { return size_; }
    int& operator[](size_t index) { return data_[index]; }
    const int& operator[](size_t index) const { return data_[index]; }
};
```

### 💡 Why the Self-Assignment Guard `if (this == &other)` is Mandatory:
If someone writes `a = a;` without this guard:
1. `delete[] data_;` destroys Object A's own memory buffer.
2. `std::copy(other.data_, ...)` attempts to read from `other.data_` (which is `data_`, now already freed!).
3. Reads garbage or triggers a segmentation fault (Use-After-Free). Always guard self-assignment!

---

## 3. The Rule of Three vs. Rule of Five vs. Rule of Zero

- **The Rule of Three (C++98 / C++03 / C++14)**:
  If your class manages a raw resource (heap memory, file descriptor) and defines any one of:
  1. Destructor
  2. Copy Constructor
  3. Copy Assignment Operator
  You **MUST define all three**.
- **The Rule of Five (C++11 / C++14 / Modern C++)**:
  With move semantics, add:
  4. Move Constructor
  5. Move Assignment Operator
- **The Rule of Zero (Modern C++ Best Practice)**:
  Avoid writing custom destructors or copy/move constructors altogether! Use standard RAII vocabulary types:
  ```cpp
  // ✅ Rule of Zero: Compiler generates perfect copy, move, and destruction automatically!
  class ModernBuffer {
      std::vector<int> data_; // Handles its own deep copy, move, and cleanup!
      std::string name_;
  };
  ```

---

## 4. The Idiomatic Copy-and-Swap Pattern
```cpp
class Buffer {
    char* data_;
    size_t size_;
public:
    // Destructor
    ~Buffer() { delete[] data_; }
    // Move Constructor
    Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    // Unified operator= handles BOTH copy and move!
    Buffer& operator=(Buffer other) noexcept { // Pass by value
        swap(*this, other);
        return *this;
    }
    friend void swap(Buffer& a, Buffer& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }
};
```

---

## 3. `std::move` vs `std::forward`
- `std::move(x)`: An unconditional cast to `T&&` (rvalue reference). It signals: *"I don't need this object anymore, steal its guts."*
- `std::forward<T>(x)`: A conditional cast used exclusively in template forwarding references (`T&&`). Casts to rvalue *only* if the original argument was an rvalue.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [01: Memory Layout & Pointers](../01-memory-layout-and-pointers/README.md) | [Modern C++ Hub](../README.md) | **[03: Smart Pointers Internals](../03-smart-pointers-internals/README.md)** |
<!-- /Chapter Navigation Footer -->

