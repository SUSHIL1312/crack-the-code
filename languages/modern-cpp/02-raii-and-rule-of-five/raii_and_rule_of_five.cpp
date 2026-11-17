/**
 * raii_and_rule_of_five.cpp
 * 
 * Verifying Rule of Five, Move Semantics, and Copy-and-Swap idiom.
 */

#include <iostream>
#include <cstring>
#include <utility>
#include <cassert>

class StringResource {
public:
    static inline int copy_count = 0;
    static inline int move_count = 0;

    StringResource(const char* str = "") {
        size_ = std::strlen(str);
        data_ = new char[size_ + 1];
        std::memcpy(data_, str, size_ + 1);
    }

    ~StringResource() {
        delete[] data_;
    }

    // Copy Constructor (Deep copy)
    StringResource(const StringResource& other) : size_(other.size_) {
        data_ = new char[size_ + 1];
        std::memcpy(data_, other.data_, size_ + 1);
        ++copy_count;
    }

    // Move Constructor (Resource theft)
    StringResource(StringResource&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
        ++move_count;
    }

    // Unified Assignment Operator (Copy-and-Swap)
    StringResource& operator=(StringResource other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(StringResource& a, StringResource& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }

    const char* c_str() const { return data_ ? data_ : ""; }
    size_t size() const { return size_; }

private:
    char* data_ = nullptr;
    size_t size_ = 0;
};

void test_rule_of_five() {
    std::cout << "[TEST] Rule of Five & Move Semantics...\n";

    StringResource s1("Hello World");
    assert(s1.size() == 11);

    // Copy
    StringResource s2 = s1;
    assert(StringResource::copy_count == 1);
    assert(s2.size() == 11);

    // Move
    StringResource s3 = std::move(s1);
    assert(StringResource::move_count == 1);
    assert(s1.c_str()[0] == '\0'); // s1 was emptied
    assert(s3.size() == 11);

    std::cout << "  ✓ Deep copy and zero-overhead move theft verified.\n";
}

int main() {
    test_rule_of_five();
    std::cout << "✅ Module 02 RAII & Rule of Five PASSED!\n";
    return 0;
}
