/**
 * @file dining_philosophers.cpp
 * @brief Demonstrates Dijkstra's Classic Dining Philosophers problem and its robust,
 *        deadlock-free resolution using Modern C++ std::scoped_lock (C++17).
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic dining_philosophers.cpp -o dining_phil
 *   ./dining_phil
 */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

constexpr int NUM_PHILOSOPHERS = 5;
constexpr int MEALS_PER_PHILOSOPHER = 3;

struct Fork {
    std::mutex mutex;
};

// Global array of forks between adjacent philosophers
std::vector<Fork> g_forks(NUM_PHILOSOPHERS);
std::mutex g_io_mutex; // For clean console printing

void philosopher_lifecycle(int id) {
    int left_fork_idx = id;
    int right_fork_idx = (id + 1) % NUM_PHILOSOPHERS;

    for (int meal = 0; meal < MEALS_PER_PHILOSOPHER; ++meal) {
        // 1. Thinking
        {
            std::lock_guard<std::mutex> io_lock(g_io_mutex);
            std::cout << "Philosopher " << id << " is THINKING.\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // 2. Eating: DEADLOCK-FREE LOCK ACQUISITION
        // std::scoped_lock uses a deadlock avoidance algorithm (similar to std::lock)
        // to atomically acquire all supplied mutexes regardless of ordering!
        {
            std::scoped_lock lock(g_forks[left_fork_idx].mutex, g_forks[right_fork_idx].mutex);

            {
                std::lock_guard<std::mutex> io_lock(g_io_mutex);
                std::cout << "   Philosopher " << id << " ACQUIRED forks " 
                          << left_fork_idx << " and " << right_fork_idx 
                          << " and is EATING (Meal " << (meal + 1) << ").\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        } // Mutexes unlocked automatically here via RAII

        {
            std::lock_guard<std::mutex> io_lock(g_io_mutex);
            std::cout << "Philosopher " << id << " RELEASED forks.\n";
        }
    }
}

int main() {
    std::cout << "Starting Dining Philosophers simulation with " 
              << NUM_PHILOSOPHERS << " philosophers...\n\n";

    std::vector<std::thread> philosophers;
    philosophers.reserve(NUM_PHILOSOPHERS);

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers.emplace_back(philosopher_lifecycle, i);
    }

    for (auto& t : philosophers) {
        t.join();
    }

    std::cout << "\nAll philosophers completed their meals without deadlocks!\n";
    return 0;
}
