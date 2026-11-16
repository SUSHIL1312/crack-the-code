/**
 * @file benchmark_context_switch.cpp
 * @brief High-precision benchmark measuring process context switch latency
 *        via bidirectional pipe ping-pong (lmbench methodology).
 *
 * Compilation:
 *   clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic benchmark_context_switch.cpp -o cs_bench
 *   ./cs_bench
 */

#include <iostream>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    constexpr int ROUND_TRIPS = 50'000;
    // Each round trip involves TWO context switches:
    // Parent writes -> blocks on read -> OS switches to Child ->
    // Child reads -> writes back -> blocks on read -> OS switches to Parent.
    constexpr int TOTAL_SWITCHES = ROUND_TRIPS * 2;

    int pipe1[2]; // Parent -> Child
    int pipe2[2]; // Child -> Parent

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        std::cerr << "Failed to create pipes\n";
        return 1;
    }

    std::cout << "Benchmarking " << TOTAL_SWITCHES << " process context switches via pipe ping-pong...\n";

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Fork failed\n";
        return 1;
    } 
    else if (pid == 0) {
        // Child Process
        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2

        char token = 'x';
        for (int i = 0; i < ROUND_TRIPS; ++i) {
            // Read from parent (blocks and yields CPU)
            if (read(pipe1[0], &token, 1) != 1) break;
            // Write to parent (wakes up parent)
            if (write(pipe2[1], &token, 1) != 1) break;
        }

        close(pipe1[0]);
        close(pipe2[1]);
        _exit(0);
    } 
    else {
        // Parent Process
        close(pipe1[0]); // Close read end of pipe1
        close(pipe2[1]); // Close write end of pipe2

        char token = 'x';

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < ROUND_TRIPS; ++i) {
            // Write to child (wakes up child)
            if (write(pipe1[1], &token, 1) != 1) break;
            // Read from child (blocks and yields CPU)
            if (read(pipe2[0], &token, 1) != 1) break;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        waitpid(pid, nullptr, 0); // Reap child

        close(pipe1[1]);
        close(pipe2[0]);

        double avg_switch_us = static_cast<double>(duration_us) / TOTAL_SWITCHES;

        std::cout << "\n================ CONTEXT SWITCH BENCHMARK ================\n";
        std::cout << "Total Elapsed Time: " << duration_us / 1000.0 << " ms\n";
        std::cout << "Total Context Switches: " << TOTAL_SWITCHES << "\n";
        std::cout << "Average Latency per Switch: " << avg_switch_us << " microseconds\n";
        std::cout << "                            (~" << avg_switch_us * 1000.0 << " nanoseconds)\n";
        std::cout << "==========================================================\n";
    }

    return 0;
}
