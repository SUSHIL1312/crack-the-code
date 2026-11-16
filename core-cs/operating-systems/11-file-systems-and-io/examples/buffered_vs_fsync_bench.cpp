/**
 * @file buffered_vs_fsync_bench.cpp
 * @brief Benchmark quantifying the dramatic latency gap between buffered page cache writes
 *        versus durable disk flushes using fsync().
 *
 * Compilation:
 *   clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic buffered_vs_fsync_bench.cpp -o fsync_bench
 *   ./fsync_bench
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>

int main() {
    constexpr int ITERATIONS = 200;
    const char data[] = "Benchmark transaction log record entry: balance update +100\n";
    constexpr size_t LEN = sizeof(data) - 1;

    // --- TEST 1: BUFFERED PAGE-CACHE WRITES ---
    int fd_buffered = open("/tmp/bench_buffered.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_buffered < 0) return 1;

    auto start_buffered = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        [[maybe_unused]] auto res = write(fd_buffered, data, LEN);
    }
    auto end_buffered = std::chrono::high_resolution_clock::now();
    auto buffered_us = std::chrono::duration_cast<std::chrono::microseconds>(end_buffered - start_buffered).count();
    close(fd_buffered);

    // --- TEST 2: DURABLE WRITES WITH FSYNC() ---
    int fd_fsync = open("/tmp/bench_fsync.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_fsync < 0) return 1;

    auto start_fsync = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        [[maybe_unused]] auto res = write(fd_fsync, data, LEN);
        fsync(fd_fsync); // Force physical hardware flush to non-volatile storage
    }
    auto end_fsync = std::chrono::high_resolution_clock::now();
    auto fsync_us = std::chrono::duration_cast<std::chrono::microseconds>(end_fsync - start_fsync).count();
    close(fd_fsync);

    // Results
    double avg_buffered_us = static_cast<double>(buffered_us) / ITERATIONS;
    double avg_fsync_us = static_cast<double>(fsync_us) / ITERATIONS;

    std::cout << "================ BUFFERED WRITE VS FSYNC BENCHMARK ================\n";
    std::cout << "Iterations: " << ITERATIONS << " sequential 60-byte log records\n\n";

    std::cout << "Buffered Writes (Page Cache):\n";
    std::cout << "  Total Time: " << buffered_us / 1000.0 << " ms\n";
    std::cout << "  Average per write: " << avg_buffered_us << " microseconds\n\n";

    std::cout << "Durable Writes (write + fsync):\n";
    std::cout << "  Total Time: " << fsync_us / 1000.0 << " ms\n";
    std::cout << "  Average per write: " << avg_fsync_us << " microseconds (" 
              << avg_fsync_us / 1000.0 << " ms)\n\n";

    std::cout << "Performance Ratio: fsync() is ~" << (avg_fsync_us / avg_buffered_us) 
              << "x SLOWER than buffered write!\n";
    std::cout << "===================================================================\n";

    // Clean up
    unlink("/tmp/bench_buffered.log");
    unlink("/tmp/bench_fsync.log");
    return 0;
}
