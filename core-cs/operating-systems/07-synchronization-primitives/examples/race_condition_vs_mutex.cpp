/**
 * @file race_condition_vs_mutex.cpp
 * @brief Demonstrates a real-world Race Condition with corrupted counter values
 *        and its proper resolution using std::mutex, std::lock_guard, and std::atomic.
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic race_condition_vs_mutex.cpp -o race_demo
 *   ./race_demo
 */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

constexpr int NUM_THREADS = 10;
constexpr int INCREMENTS_PER_THREAD = 100'000;
constexpr int EXPECTED_TOTAL = NUM_THREADS * INCREMENTS_PER_THREAD; // 1,000,000

// 1. UNSYNCHRONIZED COUNTER (Vulnerable to Race Conditions!)
int g_unsafe_counter = 0;

void unsafe_increment() {
    for (int i = 0; i < INCREMENTS_PER_THREAD; ++i) {
        // NON-ATOMIC OPERATION:
        // Compiles into: read from memory -> increment register -> write back to memory.
        // Multiple threads interleave these steps, overwriting each other's updates!
        g_unsafe_counter++;
    }
}

// 2. MUTEX-PROTECTED COUNTER (Guaranteed Mutual Exclusion via Lock Guard)
int g_safe_counter = 0;
std::mutex g_counter_mutex;

void safe_mutex_increment() {
    for (int i = 0; i < INCREMENTS_PER_THREAD; ++i) {
        // std::lock_guard provides RAII-based locking.
        // Automatically releases the mutex upon leaving the scope!
        std::lock_guard<std::mutex> lock(g_counter_mutex);
        g_safe_counter++;
    }
}

// 3. ATOMIC COUNTER (Lock-Free Hardware Primitive)
std::atomic<int> g_atomic_counter{0};

void atomic_increment() {
    for (int i = 0; i < INCREMENTS_PER_THREAD; ++i) {
        // Hardware-enforced atomic instruction (e.g., 'lock inc' on x86)
        g_atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::cout << "Target: " << NUM_THREADS << " threads, " << INCREMENTS_PER_THREAD 
              << " increments each. Expected Total = " << EXPECTED_TOTAL << "\n\n";

    // --- TEST 1: Unsynchronized ---
    {
        std::vector<std::thread> threads;
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back(unsafe_increment);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "1. Unsynchronized Counter Result: " << g_unsafe_counter << "\n";
        std::cout << "   Lost updates: " << (EXPECTED_TOTAL - g_unsafe_counter) 
                  << " (DATA CORRUPTION OCCURRED!)\n\n";
    }

    // --- TEST 2: std::mutex ---
    {
        std::vector<std::thread> threads;
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back(safe_mutex_increment);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "2. Mutex Protected Counter Result: " << g_safe_counter << "\n";
        std::cout << "   Status: " << (g_safe_counter == EXPECTED_TOTAL ? "SUCCESS (Correct)" : "FAILED") << "\n\n";
    }

    // --- TEST 3: std::atomic ---
    {
        std::vector<std::thread> threads;
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back(atomic_increment);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "3. Atomic Counter Result: " << g_atomic_counter.load() << "\n";
        std::cout << "   Status: " << (g_atomic_counter.load() == EXPECTED_TOTAL ? "SUCCESS (Correct)" : "FAILED") << "\n";
    }

    return 0;
}
