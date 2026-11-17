/**
 * @file deep_vs_shallow_copy.cpp
 * @brief Demonstrates Copy Constructor, Shallow Copy vs. Deep Copy, and
 *        Copy Assignment Operator in pure C++14.
 *
 * Compilation:
 *   clang++ -std=c++14 -Wall -Wextra -Wpedantic deep_vs_shallow_copy.cpp -o deep_shallow_demo
 *   ./deep_shallow_demo
 */

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstddef>

// ============================================================================
// PART 1: THE SHALLOW COPY HAZARD (Default Memberwise Copy)
// ============================================================================
class ShallowBuffer {
public:
    int* data;
    size_t size;

    explicit ShallowBuffer(size_t s)
        : data(s > 0 ? new int[s]() : nullptr), size(s) {}

    // Compiler-generated copy constructor does a SHALLOW copy:
    // ShallowBuffer(const ShallowBuffer& other) : data(other.data), size(other.size) {}

    // NOTE: If we put 'delete[] data;' in destructor, creating a copy causes
    // a double-free SIGABRT crash because both instances point to the same memory!
    void free_manual() {
        delete[] data;
        data = nullptr;
    }
};

// ============================================================================
// PART 2: THE DEEP COPY IMPLEMENTATION (C++14 Production Pattern)
// ============================================================================
class DeepBuffer {
private:
    int* data_;
    size_t size_;

public:
    // 1. Parameterized Constructor
    explicit DeepBuffer(size_t size)
        : data_(size > 0 ? new int[size]() : nullptr), size_(size) {}

    // 2. Destructor (RAII: cleans up dynamically allocated heap memory)
    ~DeepBuffer() {
        delete[] data_;
    }

    // 3. Deep Copy Constructor (C++14)
    // Parameter MUST be 'const DeepBuffer&' (pass-by-reference) to prevent infinite recursion!
    DeepBuffer(const DeepBuffer& other)
        : data_(other.size_ > 0 ? new int[other.size_] : nullptr),
          size_(other.size_) {
        std::cout << "  [DeepBuffer] Copy Constructor called! Allocating new heap memory...\n";
        if (data_ && other.data_) {
            std::copy(other.data_, other.data_ + other.size_, data_);
        }
    }

    // 4. Deep Copy Assignment Operator (C++14)
    DeepBuffer& operator=(const DeepBuffer& other) {
        std::cout << "  [DeepBuffer] Copy Assignment Operator called!\n";

        // STEP 1: Guard against Self-Assignment (e.g. b = b;)
        if (this == &other) {
            std::cout << "  [DeepBuffer] Self-assignment detected; skipping reallocation.\n";
            return *this;
        }

        // STEP 2: Allocate new buffer FIRST for Exception Safety
        int* new_data = other.size_ > 0 ? new int[other.size_] : nullptr;
        if (new_data && other.data_) {
            std::copy(other.data_, other.data_ + other.size_, new_data);
        }

        // STEP 3: Delete existing resource safely
        delete[] data_;

        // STEP 4: Transfer pointer & update size
        data_ = new_data;
        size_ = other.size_;

        return *this; // Return reference for chained assignment (a = b = c)
    }

    // Member accessors
    size_t size() const { return size_; }
    int* raw_pointer() const { return data_; }
    
    int& operator[](size_t idx) { return data_[idx]; }
    const int& operator[](size_t idx) const { return data_[idx]; }
};

int main() {
    std::cout << "========================================================\n";
    std::cout << "  C++14 Deep Copy vs. Shallow Copy & Copy Constructor   \n";
    std::cout << "========================================================\n\n";

    // ------------------------------------------------------------------------
    // TEST 1: Demonstrating Shallow Copy Pointer Duplication
    // ------------------------------------------------------------------------
    std::cout << "--- [Test 1] Shallow Copy Pointer Sharing ---\n";
    {
        ShallowBuffer s1(5);
        s1.data[0] = 42;

        // Default shallow copy: copies raw pointer address
        ShallowBuffer s2 = s1;

        std::cout << "  s1 buffer address: " << static_cast<void*>(s1.data) << "\n";
        std::cout << "  s2 buffer address: " << static_cast<void*>(s2.data) << "\n";

        assert(s1.data == s2.data && "Shallow copy points to the exact same heap memory!");
        std::cout << "  Notice: Both s1 and s2 share identical pointer addresses!\n";

        // Mutation in s2 corrupts s1!
        s2.data[0] = 999;
        assert(s1.data[0] == 999 && "Modifying shallow copy silently corrupted original!");
        std::cout << "  [PASS] Verified shallow copy hazard: mutation in copy mutates original.\n";

        // Clean up safely once to avoid double free
        s1.free_manual();
        s2.data = nullptr; // Prevent double delete
    }
    std::cout << "\n";

    // ------------------------------------------------------------------------
    // TEST 2: Deep Copy Constructor (Independent Allocations)
    // ------------------------------------------------------------------------
    std::cout << "--- [Test 2] Deep Copy Constructor (Memory Isolation) ---\n";
    {
        DeepBuffer d1(5);
        for (size_t i = 0; i < d1.size(); ++i) {
            d1[i] = static_cast<int>(i * 10);
        }

        // Invokes Deep Copy Constructor
        DeepBuffer d2 = d1;

        std::cout << "  d1 buffer address: " << static_cast<void*>(d1.raw_pointer()) << "\n";
        std::cout << "  d2 buffer address: " << static_cast<void*>(d2.raw_pointer()) << "\n";

        assert(d1.raw_pointer() != d2.raw_pointer() && "Deep copy must allocate unique address!");
        assert(d2.size() == d1.size());
        assert(d2[2] == 20);

        // Modifying d2 must NOT affect d1
        d2[2] = 8888;
        assert(d1[2] == 20 && "Original d1 data must remain intact after d2 modification!");
        assert(d2[2] == 8888);

        std::cout << "  [PASS] Deep Copy Constructor successfully allocated independent buffer!\n";
    }
    std::cout << "\n";

    // ------------------------------------------------------------------------
    // TEST 3: Deep Copy Assignment Operator & Self-Assignment Check
    // ------------------------------------------------------------------------
    std::cout << "--- [Test 3] Copy Assignment Operator & Self-Assignment ---\n";
    {
        DeepBuffer d1(3);
        d1[0] = 100; d1[1] = 200; d1[2] = 300;

        DeepBuffer d2(1);
        d2[0] = 5;

        // Normal assignment
        d2 = d1;
        assert(d2.size() == 3);
        assert(d2[1] == 200);
        assert(d2.raw_pointer() != d1.raw_pointer());

        // Self-Assignment test (d2 = d2)
        std::cout << "  Testing self-assignment guard (d2 = d2)...\n";
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wself-assign-overloaded"
        d2 = d2;
        #pragma clang diagnostic pop

        assert(d2.size() == 3 && "Buffer size must not change on self-assignment!");
        assert(d2[1] == 200 && "Buffer data must not be destroyed on self-assignment!");
        std::cout << "  [PASS] Self-assignment safely handled without memory corruption.\n";
    }

    std::cout << "\n========================================================\n";
    std::cout << "  ALL C++14 DEEP VS SHALLOW COPY TESTS PASSED (100%)    \n";
    std::cout << "========================================================\n";

    return 0;
}
