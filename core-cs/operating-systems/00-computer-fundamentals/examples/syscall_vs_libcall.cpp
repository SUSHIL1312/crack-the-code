/**
 * @file syscall_vs_libcall.cpp
 * @brief Measures and compares the overhead of a standard user-space function call
 *        versus a direct Operating System Kernel System Call (mode switch).
 *
 * Compilation:
 *   clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic syscall_vs_libcall.cpp -o syscall_bench
 *   ./syscall_bench
 */

#include <iostream>
#include <chrono>
#include <unistd.h>
#include <sys/syscall.h>

// Simple user-space function (executes purely in CPU User Mode / Ring 3)
__attribute__((noinline)) int user_function(int val) {
    return val * 2 + 1;
}

int main() {
    constexpr int ITERATIONS = 1'000'000;

    std::cout << "Benchmarking " << ITERATIONS << " invocations...\n";

    // 1. Benchmark User-Space Function Call
    int sum = 0;
    auto start_user = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        sum += user_function(i);
    }
    auto end_user = std::chrono::high_resolution_clock::now();
    auto user_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_user - start_user).count();

    // 2. Benchmark Kernel System Call (getpid)
    // getpid() is a lightweight syscall that queries the kernel for the process ID.
    // It requires a full user-to-kernel mode switch (Ring 3 -> Ring 0 -> Ring 3).
    pid_t pid = 0;
    auto start_sys = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        pid = getpid();
    }
    auto end_sys = std::chrono::high_resolution_clock::now();
    auto sys_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_sys - start_sys).count();

    // Print Results
    double avg_user_ns = static_cast<double>(user_duration_ns) / ITERATIONS;
    double avg_sys_ns = static_cast<double>(sys_duration_ns) / ITERATIONS;

    std::cout << "\n================ BENCHMARK RESULTS ================\n";
    std::cout << "User-space function call:\n";
    std::cout << "  Total time: " << user_duration_ns / 1'000'000.0 << " ms\n";
    std::cout << "  Average per call: " << avg_user_ns << " ns\n\n";

    std::cout << "Kernel System Call (getpid):\n";
    std::cout << "  Total time: " << sys_duration_ns / 1'000'000.0 << " ms\n";
    std::cout << "  Average per call: " << avg_sys_ns << " ns\n\n";

    std::cout << "Overhead Ratio: System call is ~" << (avg_sys_ns / avg_user_ns) << "x slower than user call.\n";
    std::cout << "====================================================\n";

    // Prevent compiler from optimizing away the loops
    if (sum == 0 || pid == 0) {
        std::cout << "Dummy check\n";
    }

    return 0;
}

/*
EXPECTED OUTPUT:
Benchmarking 1000000 invocations...

================ BENCHMARK RESULTS ================
User-space function call:
  Total time: ~1.2 ms
  Average per call: ~1.2 ns

Kernel System Call (getpid):
  Total time: ~55.0 ms
  Average per call: ~55.0 ns

Overhead Ratio: System call is ~45x slower than user call.
====================================================

WHY THIS HAPPENS (INTERNAL MECHANICS):
1. User-space function call:
   - Only involves: pushing return address to stack, jumping to instruction, executing ALU ops, returning.
   - Stays entirely in user mode (Ring 3). CPU pipeline remains primed.

2. System call (getpid):
   - Invokes 'syscall' instruction.
   - Hardware switches privilege level (Ring 3 -> Ring 0).
   - CPU switches to Kernel Stack.
   - Kernel saves CPU registers (pt_regs).
   - Kernel verifies and executes sys_getpid().
   - Kernel places result in RAX register.
   - Kernel restores registers and executes 'sysret' back to Ring 3.
   - This privilege boundary transition adds ~40-100 ns of fixed overhead.
*/
