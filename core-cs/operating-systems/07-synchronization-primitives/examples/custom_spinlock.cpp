/**
 * @file custom_spinlock.cpp
 * @brief Custom user-space Spinlock implemented via std::atomic_flag with acquire-release semantics.
 *        Includes RAII lock guard and benchmarks against std::mutex for short critical sections.
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -O2 -Wall -Wextra -Wpedantic custom_spinlock.cpp -o spinlock_demo
 *   ./spinlock_demo
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

class Spinlock {
public:
    Spinlock() = default;

    // Disallow copying
    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;

    void lock() {
        // test_and_set sets the flag to true and returns previous value.
        // memory_order_acquire ensures that subsequent memory reads/writes cannot be reordered before this lock!
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // CPU pause hint: optimizes busy-waiting loop on x86/ARM by preventing pipeline thrashing
            #if defined(__x86_64__) || defined(_M_X64)
                __builtin_ia32_pause();
            #elif defined(__aarch64__)
                asm volatile("yield" ::: "memory");
            #endif
        }
    }

    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        // memory_order_release ensures all prior memory stores are visible before the flag is cleared!
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

// RAII Wrapper for Spinlock
class SpinlockGuard {
public:
    explicit SpinlockGuard(Spinlock& spinlock) : spinlock_(spinlock) {
        spinlock_.lock();
    }
    ~SpinlockGuard() {
        spinlock_.unlock();
    }
    SpinlockGuard(const SpinlockGuard&) = delete;
    SpinlockGuard& operator=(const SpinlockGuard&) = delete;

private:
    Spinlock& spinlock_;
};

int main() {
    constexpr int NUM_THREADS = 8;
    constexpr int ITERATIONS = 100'000;

    // 1. Benchmark Custom Spinlock
    Spinlock spinlock;
    int spin_counter = 0;

    auto start_spin = std::chrono::high_resolution_clock::now();
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < ITERATIONS; ++j) {
                    SpinlockGuard guard(spinlock);
                    spin_counter++;
                }
            });
        }
        for (auto& t : threads) t.join();
    }
    auto end_spin = std::chrono::high_resolution_clock::now();
    auto spin_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_spin - start_spin).count();

    // 2. Benchmark std::mutex
    std::mutex mtx;
    int mutex_counter = 0;

    auto start_mutex = std::chrono::high_resolution_clock::now();
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < ITERATIONS; ++j) {
                    std::lock_guard<std::mutex> guard(mtx);
                    mutex_counter++;
                }
            });
        }
        for (auto& t : threads) t.join();
    }
    auto end_mutex = std::chrono::high_resolution_clock::now();
    auto mutex_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_mutex - start_mutex).count();

    std::cout << "================ SPINLOCK VS MUTEX BENCHMARK ================\n";
    std::cout << "Custom Spinlock Result: " << spin_counter << " in " << spin_time_ms << " ms\n";
    std::cout << "Standard Mutex Result:  " << mutex_counter << " in " << mutex_time_ms << " ms\n";
    std::cout << "Observation: For tiny critical sections (1 increment), Spinlock avoids kernel sleep transitions.\n";
    std::cout << "=============================================================\n";

    return 0;
}
