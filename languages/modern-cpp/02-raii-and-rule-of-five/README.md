# Module 02: RAII, Rule of Five & Move Semantics

## 1. What is RAII?
**RAII (Resource Acquisition Is Initialization)** binds resource lifetime to object lifetime:
- Resource (heap memory, socket, mutex lock, file descriptor) is acquired in the **constructor**.
- Resource is unconditionally released in the **destructor**.
- Stack unwinding during exceptions guarantees zero leaks.

---

## 2. The Rule of Five (and Rule of Zero)
If a class manages an explicit resource, you must implement or delete 5 special member functions:
1. **Destructor**: Releases the resource.
2. **Copy Constructor**: Performs deep copy of the resource.
3. **Copy Assignment**: Cleans up existing resource and deep-copies source.
4. **Move Constructor**: Steals pointer from rvalue source; sets source pointer to `nullptr`.
5. **Move Assignment**: Steals resource and releases existing resource.

### The Idiomatic Copy-and-Swap Pattern:
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
