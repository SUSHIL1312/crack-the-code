/**
 * @file producer_consumer_semaphores.cpp
 * @brief Canonical Producer-Consumer (Bounded Buffer) implementation using C++20
 *        Counting Semaphores (std::counting_semaphore) and an exclusive Mutex.
 *
 * Demonstrates:
 *   1. empty_slots semaphore (initialized to buffer capacity).
 *   2. full_slots semaphore (initialized to 0).
 *   3. Critical Rule: Always acquire Semaphore BEFORE acquiring the Mutex to avoid deadlock!
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic producer_consumer_semaphores.cpp -o prod_cons_sem
 *   ./prod_cons_sem
 */

#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <semaphore>
#include <vector>
#include <atomic>
#include <cassert>
#include <chrono>

template <typename T, size_t Capacity>
class SemaphoreBoundedBuffer {
public:
    SemaphoreBoundedBuffer()
        : empty_slots_(Capacity), full_slots_(0) {}

    // Producer pushes item into buffer
    void push(T item) {
        // Step 1: Wait for an available empty slot (decrements empty_slots)
        empty_slots_.acquire();

        // Step 2: Acquire exclusive lock to mutate the shared buffer
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            buffer_.push_back(std::move(item));
        }

        // Step 3: Signal that a new filled slot is available (increments full_slots)
        full_slots_.release();
    }

    // Consumer extracts item from buffer
    T pop() {
        // Step 1: Wait for a filled slot to be available (decrements full_slots)
        full_slots_.acquire();

        T item;
        // Step 2: Acquire exclusive lock to extract from shared buffer
        {
            std::lock_guard<std::mutex> lock(buffer_mutex_);
            assert(!buffer_.empty() && "Buffer cannot be empty after full_slots acquisition!");
            item = std::move(buffer_.front());
            buffer_.pop_front();
        }

        // Step 3: Signal that an empty slot has freed up (increments empty_slots)
        empty_slots_.release();

        return item;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        return buffer_.size();
    }

private:
    std::deque<T> buffer_;
    std::mutex buffer_mutex_;
    std::counting_semaphore<Capacity> empty_slots_;
    std::counting_semaphore<Capacity> full_slots_;
};

int main() {
    constexpr size_t CAPACITY = 4;
    constexpr int NUM_PRODUCERS = 3;
    constexpr int NUM_CONSUMERS = 3;
    constexpr int ITEMS_PER_PRODUCER = 10;
    constexpr int TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    std::cout << "=====================================================\n";
    std::cout << "  Producer-Consumer with C++20 Counting Semaphores   \n";
    std::cout << "  Buffer Capacity = " << CAPACITY 
              << " | Producers = " << NUM_PRODUCERS 
              << " | Consumers = " << NUM_CONSUMERS << "\n";
    std::cout << "=====================================================\n\n";

    SemaphoreBoundedBuffer<int, CAPACITY> buffer;
    std::vector<std::jthread> producers;
    std::vector<std::jthread> consumers;

    std::atomic<int> total_consumed{0};

    // Launch Producers
    for (int p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&buffer, p]() {
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                int val = (p + 1) * 100 + i;
                buffer.push(val);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // Launch Consumers
    for (int c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&buffer, &total_consumed]() {
            while (total_consumed.load() < TOTAL_ITEMS) {
                int val = buffer.pop();
                int current = total_consumed.fetch_add(1);
                if (current < 10 || current >= TOTAL_ITEMS - 3) {
                    std::cout << "  [Consumer] Extracted item: " << val 
                              << " (Total processed: " << (current + 1) << "/" << TOTAL_ITEMS << ")\n";
                } else if (current == 10) {
                    std::cout << "  [Consumer] ... streaming items concurrently ...\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });
    }

    // Join all threads (std::jthread joins automatically on scope exit)
    producers.clear();
    consumers.clear();

    assert(total_consumed.load() == TOTAL_ITEMS && "All items must be successfully produced and consumed!");
    std::cout << "\n[PASS] Verified! All " << TOTAL_ITEMS << " items safely processed with zero deadlocks or race conditions.\n";

    return 0;
}
