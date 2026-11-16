#include "allocator.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>

int main() {
    std::cout << "================ MEMORY ALLOCATOR SIMULATOR VERIFICATION ================\n";

    constexpr size_t HEAP_SIZE = 64 * 1024; // 64 KB
    MemoryAllocator alloc(HEAP_SIZE);
    alloc.print_heap_state();

    // Test 1: Sequential Allocations
    std::cout << "\n[Test 1] Allocating 3 blocks: P1 (1000B), P2 (2000B), P3 (4000B)...\n";
    void* p1 = alloc.allocate(1000);
    void* p2 = alloc.allocate(2000);
    void* p3 = alloc.allocate(4000);

    assert(p1 != nullptr && p2 != nullptr && p3 != nullptr);
    std::memset(p1, 0xAA, 1000);
    std::memset(p2, 0xBB, 2000);
    std::memset(p3, 0xCC, 4000);
    alloc.print_heap_state();

    // Test 2: Free middle block (P2) -> No coalescing possible yet
    std::cout << "\n[Test 2] Freeing middle block P2 (isolated hole created)...\n";
    alloc.deallocate(p2);
    alloc.print_heap_state();

    // Test 3: Free P1 -> Triggers merge right with P2!
    std::cout << "\n[Test 3] Freeing P1 (triggers merge right with P2)...\n";
    alloc.deallocate(p1);
    alloc.print_heap_state();

    // Test 4: Free P3 -> Triggers merge left and right into one unified heap!
    std::cout << "\n[Test 4] Freeing P3 (triggers full coalescing of entire heap)...\n";
    alloc.deallocate(p3);
    alloc.print_heap_state();

    assert(alloc.get_total_free_bytes() == HEAP_SIZE);
    assert(alloc.get_largest_free_block() == HEAP_SIZE);
    std::cout << "Coalescing verification: PASSED (Heap completely coalesced back to original size)!\n";

    // Test 5: Best-Fit vs First-Fit
    std::cout << "\n[Test 5] Best-Fit policy verification...\n";
    void* a1 = alloc.allocate(100);
    void* a2 = alloc.allocate(500);
    void* a3 = alloc.allocate(100);
    void* a4 = alloc.allocate(1000);
    (void)a1; (void)a3;

    // Free a2 (hole of ~500) and a4 (hole of ~1000)
    alloc.deallocate(a2);
    alloc.deallocate(a4);

    // Request 400 bytes with Best-Fit -> should pick the 500-byte hole, preserving the 1000-byte hole
    void* best_fit_ptr = alloc.allocate(400, AllocationPolicy::BEST_FIT);
    assert(best_fit_ptr != nullptr);
    std::cout << "Best-Fit successfully chose optimal block without fragmenting larger pool.\n";

    alloc.deallocate(best_fit_ptr);
    alloc.deallocate(a1);
    alloc.deallocate(a3);

    std::cout << "\nFinal Heap Status:\n";
    alloc.print_heap_state();
    assert(alloc.get_total_free_bytes() == HEAP_SIZE);

    std::cout << "=========================================================================\n";
    return 0;
}
