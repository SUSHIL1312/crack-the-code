/**
 * @file false_sharing_benchmark.cpp
 * @brief High-precision benchmark demonstrating the severe performance penalty of False Sharing
 *        and its resolution using cache line padding (alignas(64)).
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -O3 -Wall -Wextra -Wpedantic false_sharing_benchmark.cpp -o fs_bench
 *   ./fs_bench
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <new>

constexpr uint64_t ITERATIONS = 40'000'000;

// ======================== 1. FALSE SHARING STRUCT ========================
// 'a' and 'b' reside side-by-side in memory on the EXACT SAME 64-byte cache line!
struct FalseSharingStruct {
    std::atomic<uint64_t> a{0};
    std::atomic<uint64_t> b{0};
};

// ======================== 2. PADDED CACHE-ALIGNED STRUCT =================
// 64-byte alignment guarantees each variable occupies its own dedicated cache line!
struct alignas(64) PaddedStruct {
    std::atomic<uint64_t> val{0};
};

int main() {
    std::cout << "Benchmarking False Sharing across 2 threads (" << ITERATIONS << " writes each)...\n\n";

    // --- TEST 1: FALSE SHARING ---
    FalseSharingStruct bad_data;
    auto start_bad = std::chrono::high_resolution_clock::now();
    {
        std::thread t1([&]() {
            for (uint64_t i = 0; i < ITERATIONS; ++i) {
                bad_data.a.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (uint64_t i = 0; i < ITERATIONS; ++i) {
                bad_data.b.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();
    }
    auto end_bad = std::chrono::high_resolution_clock::now();
    auto bad_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_bad - start_bad).count();

    // --- TEST 2: PADDED CACHE ALIGNED ---
    PaddedStruct good_a;
    PaddedStruct good_b;
    auto start_good = std::chrono::high_resolution_clock::now();
    {
        std::thread t1([&]() {
            for (uint64_t i = 0; i < ITERATIONS; ++i) {
                good_a.val.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (uint64_t i = 0; i < ITERATIONS; ++i) {
                good_b.val.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();
    }
    auto end_good = std::chrono::high_resolution_clock::now();
    auto good_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_good - start_good).count();

    std::cout << "================ FALSE SHARING BENCHMARK ================\n";
    std::cout << "With False Sharing (Same Cache Line):  " << bad_ms << " ms\n";
    std::cout << "With Cache Padding (alignas(64)):      " << good_ms << " ms\n";
    if (good_ms > 0) {
        std::cout << "Speedup Factor: Cache padding is ~" 
                  << (static_cast<double>(bad_ms) / good_ms) << "x FASTER!\n";
    }
    std::cout << "=========================================================\n";

    return 0;
}
