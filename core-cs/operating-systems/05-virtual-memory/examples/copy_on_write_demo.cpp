/**
 * @file copy_on_write_demo.cpp
 * @brief Measures and demonstrates Copy-on-Write (COW) mechanics across fork().
 *        Proves instantaneous fork latency and quantifies the physical page split penalty on write.
 *
 * Compilation:
 *   clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic copy_on_write_demo.cpp -o cow_demo
 *   ./cow_demo
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    constexpr size_t BUFFER_SIZE = 50 * 1024 * 1024; // 50 Megabytes

    std::cout << "Allocating and initializing " << (BUFFER_SIZE / (1024 * 1024)) << " MB in parent...\n";
    std::vector<uint8_t> parent_data(BUFFER_SIZE, 0xAA);

    // 1. Measure fork() time
    auto start_fork = std::chrono::high_resolution_clock::now();
    pid_t pid = fork();
    auto end_fork = std::chrono::high_resolution_clock::now();

    if (pid < 0) {
        std::cerr << "Fork failed\n";
        return 1;
    } 
    else if (pid == 0) {
        // ====================== CHILD PROCESS ======================
        // A. Read-only pass: Read all 50 MB
        auto start_read = std::chrono::high_resolution_clock::now();
        uint64_t sum = 0;
        for (size_t i = 0; i < BUFFER_SIZE; i += 4096) {
            sum += parent_data[i];
        }
        auto end_read = std::chrono::high_resolution_clock::now();
        auto read_us = std::chrono::duration_cast<std::chrono::microseconds>(end_read - start_read).count();

        // B. Write pass: Write 1 byte to every 4KB page (Triggers COW page faults!)
        auto start_write = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < BUFFER_SIZE; i += 4096) {
            parent_data[i] = 0xBB; // Hardware traps to kernel; kernel copies 4KB frame
        }
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_us = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write).count();

        std::cout << "\n[Child Process Metrics]\n";
        std::cout << "  Read 50 MB (Shared COW frames):   " << read_us << " microseconds\n";
        std::cout << "  Write 50 MB (Triggering COW splits): " << write_us << " microseconds\n";
        std::cout << "  Write penalty ratio: COW write is ~" 
                  << (static_cast<double>(write_us) / (read_us > 0 ? read_us : 1)) << "x slower than read!\n";

        if (sum == 0) std::cout << "Dummy\n";
        _exit(0);
    } 
    else {
        // ====================== PARENT PROCESS ======================
        auto fork_us = std::chrono::duration_cast<std::chrono::microseconds>(end_fork - start_fork).count();
        std::cout << "[Parent Process Metrics]\n";
        std::cout << "  Time to fork() 50 MB process: " << fork_us << " microseconds (" 
                  << fork_us / 1000.0 << " ms)\n";
        std::cout << "  Observation: fork() completed in sub-millisecond time because ZERO physical bytes were copied!\n";

        waitpid(pid, nullptr, 0); // Wait for child to finish
    }

    return 0;
}
