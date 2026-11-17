/**
 * smart_pointers_internals.cpp
 * 
 * Verifying unique_ptr EBO, shared_ptr ref counts, and weak_ptr cycle resolution.
 */

#include <iostream>
#include <memory>
#include <cassert>

struct CustomDeleter {
    void operator()(int* p) const {
        delete p;
    }
};

struct NodeB;

struct NodeA {
    std::shared_ptr<NodeB> b_ptr;
    ~NodeA() { std::cout << "  ✓ NodeA destroyed\n"; }
};

struct NodeB {
    std::weak_ptr<NodeA> a_weak; // weak_ptr breaks cycle!
    ~NodeB() { std::cout << "  ✓ NodeB destroyed\n"; }
};

void test_unique_ptr_size() {
    std::cout << "[TEST] std::unique_ptr Empty Base Optimization (EBO)...\n";
    std::unique_ptr<int> u1(new int(10));
    std::unique_ptr<int, CustomDeleter> u2(new int(20));

    // Zero overhead: sizeof unique_ptr matches raw pointer
    assert(sizeof(u1) == sizeof(void*));
    assert(sizeof(u2) == sizeof(void*));
    std::cout << "  ✓ sizeof(unique_ptr) == " << sizeof(u1) << " bytes (Zero overhead).\n";
}

void test_cycle_breaking() {
    std::cout << "[TEST] Breaking Circular References with weak_ptr...\n";
    {
        auto a = std::make_shared<NodeA>();
        auto b = std::make_shared<NodeB>();
        a->b_ptr = b;
        b->a_weak = a; // Weak reference does not increment use_count

        assert(a.use_count() == 1);
        assert(b.use_count() == 2); // b is held by local var 'b' and 'a->b_ptr'

        // Locking weak pointer produces valid shared_ptr
        auto locked_a = b->a_weak.lock();
        assert(locked_a != nullptr);
        assert(a.use_count() == 2);
    }
    std::cout << "  ✓ Circular reference resolved cleanly without memory leak.\n";
}

int main() {
    test_unique_ptr_size();
    test_cycle_breaking();
    std::cout << "✅ Module 03 Smart Pointers PASSED!\n";
    return 0;
}
