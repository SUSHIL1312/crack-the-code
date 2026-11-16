#include "thread_pool.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <cassert>

int main() {
    std::cout << "================ C++20 THREAD POOL VERIFICATION ================\n";

    ThreadPool pool(4);
    std::cout << "Initialized thread pool with " << pool.size() << " worker threads.\n";

    // Test 1: Submitting tasks with return values
    std::cout << "\n[Test 1] Parallel mathematical computations...\n";
    constexpr int NUM_TASKS = 20;
    std::vector<std::future<uint64_t>> futures;

    for (int i = 0; i < NUM_TASKS; ++i) {
        futures.push_back(pool.submit([i]() -> uint64_t {
            uint64_t sum = 0;
            for (uint64_t k = 0; k < 100'000; ++k) {
                sum += (i * 1000 + k);
            }
            return sum;
        }));
    }

    uint64_t total_sum = 0;
    for (auto& fut : futures) {
        total_sum += fut.get();
    }
    std::cout << "Successfully retrieved all results! Total sum: " << total_sum << "\n";

    // Test 2: Exception handling and propagation
    std::cout << "\n[Test 2] Exception propagation across thread boundary...\n";
    auto error_future = pool.submit([]() -> int {
        throw std::invalid_argument("Intentional exception thrown from worker thread!");
    });

    try {
        error_future.get();
        assert(false && "Should have thrown exception!");
    } catch (const std::invalid_argument& ex) {
        std::cout << "Caught expected exception from worker: \"" << ex.what() << "\"\n";
    }

    // Test 3: Throughput Benchmark
    std::cout << "\n[Test 3] High-throughput task submission benchmark (100,000 tasks)...\n";
    constexpr int BENCH_TASKS = 100'000;
    std::atomic<int> completed{0};

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCH_TASKS; ++i) {
        pool.submit([&completed]() {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait until all tasks execute
    while (completed.load(std::memory_order_relaxed) < BENCH_TASKS) {
        std::this_thread::yield();
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Executed " << BENCH_TASKS << " tasks in " << total_ms << " ms ("
              << (BENCH_TASKS / (total_ms / 1000.0)) << " tasks/sec)\n";

    // Test 4: Graceful Shutdown
    std::cout << "\n[Test 4] Graceful shutdown...\n";
    pool.shutdown();
    std::cout << "Pool shut down cleanly.\n";

    std::cout << "=================================================================\n";
    return 0;
}
