/**
 * @file benchmark_process_vs_thread.cpp
 * @brief Measures the latency difference between spawning and reaping a process (fork + waitpid)
 *        versus spawning and joining a thread (std::thread).
 *
 * Compilation:
 *   clang++ -std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic benchmark_process_vs_thread.cpp -o bench_proc_thread
 *   ./bench_proc_thread
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

void dummy_thread_work() {
    // Minimal work
}

int main() {
    constexpr int ITERATIONS = 2'000;

    std::cout << "Benchmarking " << ITERATIONS << " sequential creations...\n\n";

    // 1. Measure Process Creation (fork + exit + waitpid)
    auto start_proc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            _exit(0); // Terminate child immediately
        } else if (pid > 0) {
            waitpid(pid, nullptr, 0); // Reap child
        } else {
            std::cerr << "Fork failed at iteration " << i << "\n";
            break;
        }
    }
    auto end_proc = std::chrono::high_resolution_clock::now();
    auto proc_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_proc - start_proc).count();

    // 2. Measure Thread Creation (std::thread spawn + join)
    auto start_thread = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        std::thread t(dummy_thread_work);
        t.join();
    }
    auto end_thread = std::chrono::high_resolution_clock::now();
    auto thread_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_thread - start_thread).count();

    // Summary
    double avg_proc_us = static_cast<double>(proc_time_us) / ITERATIONS;
    double avg_thread_us = static_cast<double>(thread_time_us) / ITERATIONS;

    std::cout << "================ BENCHMARK RESULTS ================\n";
    std::cout << "Process (fork + waitpid):\n";
    std::cout << "  Total Time: " << proc_time_us / 1000.0 << " ms\n";
    std::cout << "  Average per process: " << avg_proc_us << " microseconds\n\n";

    std::cout << "Thread (std::thread spawn + join):\n";
    std::cout << "  Total Time: " << thread_time_us / 1000.0 << " ms\n";
    std::cout << "  Average per thread:  " << avg_thread_us << " microseconds\n\n";

    std::cout << "Result: Thread creation is ~" << (avg_proc_us / avg_thread_us) 
              << "x FASTER than process creation.\n";
    std::cout << "====================================================\n";

    return 0;
}

/*
WHY ARE THREADS FASTER TO CREATE?
1. Address Space:
   - fork() must allocate a new task_struct, duplicate the page tables (even with COW),
     and configure independent memory descriptors (mm_struct).
   - pthread_create() shares the caller's mm_struct, files_struct, and page tables directly.
     It only needs to allocate a thread stack and a small thread control block (TCB).

2. Context Switch & TLB:
   - When switching to a newly forked child process, the CPU must switch page directories (CR3/TTBR),
     invalidating TLB entries.
   - When switching between threads in the same process, the memory map stays intact (no TLB flush).
*/
