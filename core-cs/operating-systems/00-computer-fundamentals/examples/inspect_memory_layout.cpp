/**
 * @file inspect_memory_layout.cpp
 * @brief Demonstrates and prints virtual memory addresses for distinct process segments:
 *        .text (code), .rodata (constants), .data, .bss, heap, and stack.
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic inspect_memory_layout.cpp -o inspect_memory
 *   ./inspect_memory
 */

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdint>

// Global variables located in .data (initialized to non-zero)
int g_initialized_var = 42;

// Global variable located in .bss (uninitialized or initialized to 0)
int g_uninitialized_var;

// Read-only constant located in .rodata
const char* const g_rodata_string = "Hello, Memory Segments!";

void dummy_function() {
    // Compiled instructions live in .text segment
}

int main(int argc, char* argv[]) {
    // Local variables reside on the STACK
    int local_stack_var = 100;

    // Dynamically allocated memory resides on the HEAP
    auto heap_obj = std::make_unique<int>(200);

    std::cout << "================ PROCESS MEMORY LAYOUT ================\n";
    std::cout << "[Segment]      [Variable/Symbol]        [Virtual Address]\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << ".text (code)   dummy_function:          " << reinterpret_cast<void*>(&dummy_function) << "\n";
    std::cout << ".rodata        g_rodata_string literal: " << static_cast<const void*>(g_rodata_string) << "\n";
    std::cout << ".data (init)   g_initialized_var:       " << static_cast<void*>(&g_initialized_var) << "\n";
    std::cout << ".bss (uninit)  g_uninitialized_var:     " << static_cast<void*>(&g_uninitialized_var) << "\n";
    std::cout << "Heap           heap_obj.get():          " << static_cast<void*>(heap_obj.get()) << "\n";
    std::cout << "Stack          local_stack_var:         " << static_cast<void*>(&local_stack_var) << "\n";
    std::cout << "Stack (argc)   &argc:                   " << static_cast<void*>(&argc) << "\n";
    std::cout << "========================================================\n\n";

    // On Linux systems, inspect /proc/self/maps to examine memory mappings directly
    std::ifstream maps("/proc/self/maps");
    if (maps.is_open()) {
        std::cout << "--- First 10 entries of /proc/self/maps (Linux Virtual Memory Mappings) ---\n";
        std::string line;
        int count = 0;
        while (std::getline(maps, line) && count < 10) {
            std::cout << line << "\n";
            count++;
        }
    } else {
        std::cout << "Note: /proc/self/maps is available natively on Linux.\n"
                  << "On macOS, virtual memory regions can be inspected via: vmmap " 
                  << argv[0] << " in the terminal.\n";
    }

    return 0;
}

/*
EXPECTED OUTPUT (Addresses vary by system and ASLR - Address Space Layout Randomization):
================ PROCESS MEMORY LAYOUT ================
[Segment]      [Variable/Symbol]        [Virtual Address]
--------------------------------------------------------
.text (code)   dummy_function:          0x102cb3de0
.rodata        g_rodata_string literal: 0x102cb3f40
.data (init)   g_initialized_var:       0x102cb8000
.bss (uninit)  g_uninitialized_var:     0x102cb8004
Heap           heap_obj.get():          0x600000204000
Stack          local_stack_var:         0x16d1be0ac
Stack (argc)   &argc:                   0x16d1be0bc
========================================================

OBSERVATION & INTERNAL MECHANICS:
1. Low addresses hold .text, .rodata, .data, and .bss.
2. The Heap address is significantly higher than .bss.
3. The Stack resides at very high virtual memory addresses (near 0x7fff... or 0x16d...),
   growing downwards toward the heap as stack frames are pushed.
4. ASLR (Address Space Layout Randomization) randomizes base addresses on each execution
   to prevent exploitation techniques like return-oriented programming (ROP).
*/
