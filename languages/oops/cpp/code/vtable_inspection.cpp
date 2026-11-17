#include <iostream>
#include <cassert>
#include <cstdint>

class NonVirtualClass {
public:
    int data{42};
};

class VirtualClass {
public:
    int data{100};
    virtual int get_data() const { return data; }
    virtual int compute(int factor) const { return data * factor; }
    virtual ~VirtualClass() = default;
};

class DerivedVirtual : public VirtualClass {
public:
    int get_data() const override { return data + 50; }
    int compute(int factor) const override { return (data + 50) * factor; }
};

// Function pointer signature matching get_data (this pointer in %rdi / first param)
typedef int (*GetDataFn)(const VirtualClass*);

int main() {
    std::cout << "=== Running Lab: Vtable and Vptr Memory Inspection ===\n";

    // 1. Verify sizeof differences
    assert(sizeof(NonVirtualClass) == 4);
    // On 64-bit systems: 8 bytes (vptr) + 4 bytes (int data) + 4 bytes (padding) = 16 bytes
    assert(sizeof(VirtualClass) == 16);
    assert(sizeof(DerivedVirtual) == 16);
    std::cout << "[PASS] Memory sizing and vptr presence verified (16 bytes vs 4 bytes).\n";

    // 2. Direct virtual dispatch via normal C++ pointer
    DerivedVirtual derived_obj;
    VirtualClass* base_ptr = &derived_obj;
    assert(base_ptr->get_data() == 150);
    assert(base_ptr->compute(3) == 450);
    std::cout << "[PASS] Standard dynamic dispatch verified.\n";

    // 3. Inspecting the raw vptr at offset 0
    uintptr_t* object_memory = reinterpret_cast<uintptr_t*>(base_ptr);
    uintptr_t raw_vptr = object_memory[0];
    assert(raw_vptr != 0 && "Vptr at offset 0 must be non-null");
    std::cout << "[PASS] Hidden vptr detected at offset 0: 0x" << std::hex << raw_vptr << std::dec << "\n";

    // 4. Manually dereference vtable and call slot 0 (get_data) directly through function pointer!
    uintptr_t* vtable = reinterpret_cast<uintptr_t*>(raw_vptr);
    GetDataFn manual_get_data = reinterpret_cast<GetDataFn>(vtable[0]);
    int manual_result = manual_get_data(base_ptr);
    assert(manual_result == 150);
    std::cout << "[PASS] Manual vtable extraction and indirect invocation successful: " << manual_result << "\n";

    std::cout << "All Vtable inspection tests passed successfully!\n\n";
    return 0;
}
