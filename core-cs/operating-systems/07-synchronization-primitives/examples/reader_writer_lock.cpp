/**
 * @file reader_writer_lock.cpp
 * @brief Demonstrates Reader-Writer synchronization using std::shared_mutex (C++17).
 *        Compares performance of Shared Locks vs Exclusive Mutex on read-heavy workloads.
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -O2 -Wall -Wextra -Wpedantic reader_writer_lock.cpp -o rw_bench
 *   ./rw_bench
 */

#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <unordered_map>

class ThreadSafeCacheSharedMutex {
public:
    int read(int key) {
        // Shared Lock: Multiple readers can read simultaneously!
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        auto it = map_.find(key);
        return (it != map_.end()) ? it->second : -1;
    }

    void write(int key, int value) {
        // Exclusive Lock: Only one writer permitted; blocks all readers & writers
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);
        map_[key] = value;
    }

private:
    std::unordered_map<int, int> map_;
    std::shared_mutex rw_mutex_;
};

class ThreadSafeCacheExclusiveMutex {
public:
    int read(int key) {
        // Exclusive Lock: Every single reader serializes and blocks all other readers!
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        return (it != map_.end()) ? it->second : -1;
    }

    void write(int key, int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
    }

private:
    std::unordered_map<int, int> map_;
    std::mutex mutex_;
};

int main() {
    constexpr int NUM_READERS = 8;
    constexpr int NUM_WRITERS = 1;
    constexpr int READS_PER_THREAD = 100'000;
    constexpr int WRITES_PER_THREAD = 1'000; // 99% Read-heavy workload

    std::cout << "Benchmarking 99% Read-Heavy Cache (" << NUM_READERS << " Readers, " 
              << NUM_WRITERS << " Writers)...\n\n";

    // 1. Benchmark Shared Mutex (Concurrent Readers)
    {
        ThreadSafeCacheSharedMutex cache;
        cache.write(1, 100);

        auto start = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> threads;

        for (int i = 0; i < NUM_READERS; ++i) {
            threads.emplace_back([&cache]() {
                int sum = 0;
                for (int j = 0; j < READS_PER_THREAD; ++j) {
                    sum += cache.read(1);
                }
                if (sum == 0) std::cout << "Dummy\n";
            });
        }
        for (int i = 0; i < NUM_WRITERS; ++i) {
            threads.emplace_back([&cache]() {
                for (int j = 0; j < WRITES_PER_THREAD; ++j) {
                    cache.write(1, j);
                }
            });
        }
        for (auto& t : threads) t.join();
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "std::shared_mutex (Reader-Writer Lock) Time: " << ms << " ms\n";
    }

    // 2. Benchmark Exclusive Mutex (Serialized Readers)
    {
        ThreadSafeCacheExclusiveMutex cache;
        cache.write(1, 100);

        auto start = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> threads;

        for (int i = 0; i < NUM_READERS; ++i) {
            threads.emplace_back([&cache]() {
                int sum = 0;
                for (int j = 0; j < READS_PER_THREAD; ++j) {
                    sum += cache.read(1);
                }
                if (sum == 0) std::cout << "Dummy\n";
            });
        }
        for (int i = 0; i < NUM_WRITERS; ++i) {
            threads.emplace_back([&cache]() {
                for (int j = 0; j < WRITES_PER_THREAD; ++j) {
                    cache.write(1, j);
                }
            });
        }
        for (auto& t : threads) t.join();
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "std::mutex (Exclusive Lock) Time:            " << ms << " ms\n";
    }

    std::cout << "\nTakeaway: Under read-heavy concurrency, shared locks allow parallel L1/L2 cache reads.\n";
    return 0;
}
