/**
 * polymorphism_and_vtables.cpp
 * 
 * Inspecting raw vptr layout, multiple inheritance pointer adjustment, and virtual destructors.
 */

#include <iostream>
#include <cstdint>
#include <cassert>

class BaseA {
public:
    virtual void foo() { std::cout << "BaseA::foo\n"; }
    virtual ~BaseA() = default;
    int a_val = 10;
};

class BaseB {
public:
    virtual void bar() { std::cout << "BaseB::bar\n"; }
    virtual ~BaseB() = default;
    int b_val = 20;
};

class Derived : public BaseA, public BaseB {
public:
    void foo() override { std::cout << "Derived::foo\n"; }
    void bar() override { std::cout << "Derived::bar\n"; }
    int d_val = 30;
};

void test_multiple_inheritance_pointer_adjustment() {
    std::cout << "[TEST] Multiple Inheritance Pointer Adjustment...\n";

    Derived d;
    Derived* d_ptr = &d;
    BaseA* a_ptr = d_ptr;
    BaseB* b_ptr = d_ptr;

    uintptr_t addr_d = reinterpret_cast<uintptr_t>(d_ptr);
    uintptr_t addr_a = reinterpret_cast<uintptr_t>(a_ptr);
    uintptr_t addr_b = reinterpret_cast<uintptr_t>(b_ptr);

    // BaseA aligns with the start of Derived (offset 0)
    assert(addr_d == addr_a);

    // BaseB is adjusted forward by sizeof(BaseA)
    assert(addr_b > addr_d);
    assert(addr_b - addr_d == sizeof(BaseA));

    std::cout << "  ✓ Derived address: 0x" << std::hex << addr_d << "\n";
    std::cout << "  ✓ BaseA address:   0x" << addr_a << " (offset 0)\n";
    std::cout << "  ✓ BaseB address:   0x" << addr_b << " (adjusted by +" << std::dec << sizeof(BaseA) << " bytes)\n";
}

int main() {
    test_multiple_inheritance_pointer_adjustment();
    std::cout << "✅ Module 04 Polymorphism & Vtables PASSED!\n";
    return 0;
}
