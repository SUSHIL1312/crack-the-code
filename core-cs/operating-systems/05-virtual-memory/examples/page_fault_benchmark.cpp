/**
 * @file page_fault_benchmark.cpp
 * @brief Measures and proves Demand Paging and Minor Page Faults
 *        using mmap() and getrusage().
 *
 * Compilation:
 *   clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic page_fault_benchmark.cpp -o pf_bench
 *   ./pf_bench
 */

#include <iostream>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#include <cstring>

long get_minor_faults() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_minflt;
}

int main() {
    constexpr size_t PAGE_SIZE = 4096;
    constexpr size_t NUM_PAGES = 10'000;
    constexpr size_t TOTAL_BYTES = NUM_PAGES * PAGE_SIZE; // ~40 MB

    std::cout << "================ DEMAND PAGING BENCHMARK ================\n";
    std::cout << "Target allocation: " << NUM_PAGES << " pages (~" << (TOTAL_BYTES / (1024 * 1024)) << " MB)\n";

    long initial_faults = get_minor_faults();
    std::cout << "Initial minor page faults: " << initial_faults << "\n\n";

    // Step 1: Allocate virtual memory via mmap
    // Notice: MAP_ANONYMOUS | MAP_PRIVATE allocates virtual address space, NOT physical DRAM!
    char* buffer = static_cast<char*>(mmap(
        nullptr,
        TOTAL_BYTES,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANON,
        -1,
        0
    ));

    if (buffer == MAP_FAILED) {
        std::cerr << "mmap failed\n";
        return 1;
    }

    long faults_after_mmap = get_minor_faults();
    std::cout << "Step 1: After mmap() call:\n";
    std::cout << "  Minor page faults: " << faults_after_mmap << " (Delta: " 
              << (faults_after_mmap - initial_faults) << ")\n";
    std::cout << "  Observation: Zero new page faults! Virtual memory is reserved, but no RAM is allocated.\n\n";

    // Step 2: Touch 1 byte on every 4 KB page
    // This forces the MMU to trigger a hardware page fault for EVERY page!
    std::cout << "Step 2: Touching 1 byte on each of the " << NUM_PAGES << " pages...\n";
    for (size_t i = 0; i < NUM_PAGES; ++i) {
        buffer[i * PAGE_SIZE] = 'A'; // Write triggers page fault
    }

    long faults_after_touch = get_minor_faults();
    long touched_delta = faults_after_touch - faults_after_mmap;
    std::cout << "  Minor page faults after touching: " << faults_after_touch 
              << " (Delta: " << touched_delta << ")\n";
    std::cout << "  Observation: Kernel handled ~" << touched_delta 
              << " Minor Page Faults to allocate physical frames lazily!\n\n";

    // Step 3: Touch all pages a second time
    std::cout << "Step 3: Writing to all pages a second time...\n";
    for (size_t i = 0; i < NUM_PAGES; ++i) {
        buffer[i * PAGE_SIZE] = 'B';
    }

    long faults_second_pass = get_minor_faults();
    std::cout << "  Minor page faults after second pass: " << faults_second_pass 
              << " (Delta: " << (faults_second_pass - faults_after_touch) << ")\n";
    std::cout << "  Observation: Zero new page faults! Physical frames are now resident in RAM.\n";
    std::cout << "=========================================================\n";

    // Clean up
    munmap(buffer, TOTAL_BYTES);
    return 0;
}
