#include <iostream>
#include <cassert>

static int active_allocations = 0;

class BaseWithVirtualDtor {
public:
    virtual ~BaseWithVirtualDtor() {
        // Base cleanup
    }
};

class DerivedWithVirtualDtor : public BaseWithVirtualDtor {
private:
    int* resource;
public:
    DerivedWithVirtualDtor() {
        resource = new int[500];
        active_allocations++;
    }
    ~DerivedWithVirtualDtor() override {
        delete[] resource;
        active_allocations--;
    }
};

int main() {
    std::cout << "=== Running Lab: Virtual Destructors & Leak Prevention ===\n";

    assert(active_allocations == 0);

    // Polymorphic allocation & deletion
    BaseWithVirtualDtor* poly_ptr = new DerivedWithVirtualDtor();
    assert(active_allocations == 1 && "Resource should be allocated");

    // Deleting via base pointer
    delete poly_ptr;

    // Because Base has a virtual destructor, Derived's destructor was called!
    assert(active_allocations == 0 && "Resource should be cleanly deallocated via virtual dtor");
    std::cout << "[PASS] Virtual destructor correctly invoked derived cleanup (active_allocations == 0).\n";

    std::cout << "All Virtual Destructor tests passed successfully!\n\n";
    return 0;
}
