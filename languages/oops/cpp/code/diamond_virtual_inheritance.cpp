#include <iostream>
#include <cassert>

// Part 1: Multiple Inheritance Pointer Adjustment
class InterfaceA {
public:
    int a{1};
    virtual void methodA() {}
    virtual ~InterfaceA() = default;
};

class InterfaceB {
public:
    int b{2};
    virtual void methodB() {}
    virtual ~InterfaceB() = default;
};

class CompositeClass : public InterfaceA, public InterfaceB {
public:
    int c{3};
    void methodA() override {}
    void methodB() override {}
};

// Part 2: Diamond Problem with Virtual Inheritance
class CommonBase {
public:
    int shared_id{42};
    CommonBase() = default;
    CommonBase(int id) : shared_id(id) {}
    virtual ~CommonBase() = default;
};

class BranchLeft : virtual public CommonBase {
public:
    BranchLeft() : CommonBase(100) {} // Will be bypassed in BottomClass!
};

class BranchRight : virtual public CommonBase {
public:
    BranchRight() : CommonBase(200) {} // Will be bypassed in BottomClass!
};

class BottomClass : public BranchLeft, public BranchRight {
public:
    // Virtual base must be initialized directly by the most derived class!
    BottomClass(int final_id) : CommonBase(final_id), BranchLeft(), BranchRight() {}
};

int main() {
    std::cout << "=== Running Lab: Multiple Inheritance & Diamond Virtual Inheritance ===\n";

    // 1. Verify this pointer adjustment
    CompositeClass comp;
    InterfaceA* ptrA = &comp;
    InterfaceB* ptrB = &comp;

    std::cout << "Address of comp:  " << (void*)&comp << "\n";
    std::cout << "Address of ptrA:  " << (void*)ptrA << "\n";
    std::cout << "Address of ptrB:  " << (void*)ptrB << "\n";

    assert((void*)ptrA == (void*)&comp && "InterfaceA must start at offset 0");
    assert((void*)ptrB != (void*)&comp && "InterfaceB pointer must be adjusted to non-zero offset");
    assert((uintptr_t)ptrB > (uintptr_t)ptrA && "InterfaceB offset must follow InterfaceA");
    std::cout << "[PASS] Pointer adjustment under multiple inheritance verified.\n";

    // 2. Verify Virtual Inheritance resolves the diamond problem to a single shared base instance
    BottomClass diamond(999);
    // Unambiguous direct access to shared_id without needing BranchLeft:: or BranchRight::
    assert(diamond.shared_id == 999);
    assert(diamond.BranchLeft::shared_id == 999);
    assert(diamond.BranchRight::shared_id == 999);

    // Verify mutating via one branch mutates the single shared instance
    diamond.BranchLeft::shared_id = 1234;
    assert(diamond.BranchRight::shared_id == 1234);
    assert(diamond.shared_id == 1234);
    std::cout << "[PASS] Virtual inheritance verified: single shared base instance resolved.\n";

    std::cout << "All Multiple Inheritance & Diamond tests passed successfully!\n\n";
    return 0;
}
