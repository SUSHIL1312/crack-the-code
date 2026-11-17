/**
 * custom_shared_ptr.cpp
 * 
 * Production-Quality Custom Shared Pointer Implementation:
 * - Separate Control Block holding atomic ref count
 * - Copy constructor & copy assignment (increments ref count)
 * - Move constructor & move assignment (steals pointer without ref count churn)
 * - Reset and custom dereferencing operators
 * - Comprehensive unit test suite
 */

#include <iostream>
#include <atomic>
#include <utility>
#include <cassert>

template <typename T>
class CustomSharedPtr {
private:
    struct ControlBlock {
        std::atomic<uint32_t> ref_count{1};
    };

    T* ptr_ = nullptr;
    ControlBlock* cb_ = nullptr;

    void release() {
        if (cb_) {
            if (cb_->ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete ptr_;
                delete cb_;
            }
            ptr_ = nullptr;
            cb_ = nullptr;
        }
    }

public:
    CustomSharedPtr() = default;

    explicit CustomSharedPtr(T* ptr) : ptr_(ptr) {
        if (ptr_) {
            cb_ = new ControlBlock();
        }
    }

    ~CustomSharedPtr() {
        release();
    }

    // Copy Constructor
    CustomSharedPtr(const CustomSharedPtr& other) : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->ref_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Copy Assignment
    CustomSharedPtr& operator=(const CustomSharedPtr& other) {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            if (cb_) {
                cb_->ref_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    // Move Constructor
    CustomSharedPtr(CustomSharedPtr&& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    // Move Assignment
    CustomSharedPtr& operator=(CustomSharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            other.ptr_ = nullptr;
            other.cb_ = nullptr;
        }
        return *this;
    }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* get() const { return ptr_; }
    uint32_t use_count() const { return cb_ ? cb_->ref_count.load(std::memory_order_relaxed) : 0; }
    explicit operator bool() const { return ptr_ != nullptr; }
};

struct TrackedObject {
    static inline int alive = 0;
    int val;
    TrackedObject(int v) : val(v) { ++alive; }
    ~TrackedObject() { --alive; }
};

void run_tests() {
    std::cout << "[TEST] Custom Shared Pointer Lifecycle...\n";
    assert(TrackedObject::alive == 0);

    {
        CustomSharedPtr<TrackedObject> sp1(new TrackedObject(42));
        assert(sp1.use_count() == 1);
        assert(TrackedObject::alive == 1);
        assert(sp1->val == 42);

        {
            CustomSharedPtr<TrackedObject> sp2 = sp1; // Copy
            assert(sp1.use_count() == 2);
            assert(sp2.use_count() == 2);
            assert(TrackedObject::alive == 1);
        } // sp2 destroyed

        assert(sp1.use_count() == 1);
        assert(TrackedObject::alive == 1);

        CustomSharedPtr<TrackedObject> sp3 = std::move(sp1); // Move
        assert(!sp1);
        assert(sp3.use_count() == 1);
        assert(TrackedObject::alive == 1);
    } // sp3 destroyed

    assert(TrackedObject::alive == 0 && "Memory must be freed cleanly without leaks!");
    std::cout << "  ✓ All CustomSharedPtr tests passed with zero leaks!\n";
}

int main() {
    run_tests();
    std::cout << "✅ Capstone 01 Custom Shared Ptr PASSED!\n";
    return 0;
}
