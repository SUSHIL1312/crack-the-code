/**
 * memory_layout_and_pointers.cpp
 * 
 * Verifying Struct Padding, Alignment, and Const Pointer Permutations.
 */

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cassert>

struct BadLayout {
    char a;      // 1 byte
    double b;    // 8 bytes
    char c;      // 1 byte
};

struct GoodLayout {
    double b;    // 8 bytes
    char a;      // 1 byte
    char c;      // 1 byte
};

void test_memory_layout() {
    std::cout << "[TEST] Struct Alignment & Padding...\n";
    assert(sizeof(BadLayout) == 24);
    assert(sizeof(GoodLayout) == 16);
    assert(offsetof(BadLayout, b) == 8);
    assert(offsetof(GoodLayout, a) == 8);
    assert(offsetof(GoodLayout, c) == 9);
    std::cout << "  ✓ Unoptimized struct: 24 bytes vs Optimized: 16 bytes (Saved 8 bytes per object).\n";
}

void test_pointer_permutations() {
    std::cout << "[TEST] Const Pointer Permutations...\n";
    int val1 = 10;
    int val2 = 20;

    // Pointer to const
    const int* p_to_const = &val1;
    p_to_const = &val2; // Allowed
    assert(*p_to_const == 20);

    // Const pointer
    int* const const_p = &val1;
    *const_p = 99; // Allowed
    assert(val1 == 99);

    std::cout << "  ✓ Const pointer semantics verified.\n";
}

int main() {
    test_memory_layout();
    test_pointer_permutations();
    std::cout << "✅ Module 01 Memory & Pointers PASSED!\n";
    return 0;
}
