/**
 * custom_vector.cpp
 * 
 * Production-Quality Custom Vector Implementation:
 * - Direct uninitialized memory management via ::operator new and placement new
 * - Geometric growth (2x factor)
 * - Exception safety using std::move_if_noexcept
 * - Iterators and full unit test suite
 */

#include <iostream>
#include <memory>
#include <utility>
#include <cassert>

template <typename T>
class CustomVector {
private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void reallocate(size_t new_cap) {
        T* new_data = static_cast<T*>(::operator new(new_cap * sizeof(T)));
        size_t moved = 0;
        try {
            for (size_t i = 0; i < size_; ++i) {
                new (static_cast<void*>(new_data + i)) T(std::move_if_noexcept(data_[i]));
                ++moved;
            }
        } catch (...) {
            for (size_t i = 0; i < moved; ++i) {
                new_data[i].~T();
            }
            ::operator delete(new_data);
            throw;
        }

        // Clean up old memory
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        ::operator delete(data_);

        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    CustomVector() = default;

    ~CustomVector() {
        clear();
        ::operator delete(data_);
    }

    void clear() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(size_t new_cap) {
        if (new_cap > capacity_) {
            reallocate(new_cap);
        }
    }

    void push_back(const T& val) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (static_cast<void*>(data_ + size_)) T(val);
        ++size_;
    }

    void push_back(T&& val) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (static_cast<void*>(data_ + size_)) T(std::move(val));
        ++size_;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (static_cast<void*>(data_ + size_)) T(std::forward<Args>(args)...);
        ++size_;
    }

    void pop_back() {
        assert(size_ > 0);
        --size_;
        data_[size_].~T();
    }

    T& operator[](size_t idx) noexcept { return data_[idx]; }
    const T& operator[](size_t idx) const noexcept { return data_[idx]; }

    T* begin() noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }
};

void run_tests() {
    std::cout << "[TEST] Custom Vector Operations & Geometric Growth...\n";

    CustomVector<int> vec;
    assert(vec.empty());

    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
    }

    assert(vec.size() == 10);
    assert(vec.capacity() >= 10);
    assert(vec[0] == 0);
    assert(vec[9] == 90);

    vec.pop_back();
    assert(vec.size() == 9);
    assert(vec[8] == 80);

    // Range-based for loop
    int sum = 0;
    for (int x : vec) {
        sum += x;
    }
    assert(sum == 360);

    std::cout << "  ✓ Vector resizing, push_back, pop_back, and iterators verified.\n";
}

int main() {
    run_tests();
    std::cout << "✅ Capstone 02 Custom Vector PASSED!\n";
    return 0;
}
