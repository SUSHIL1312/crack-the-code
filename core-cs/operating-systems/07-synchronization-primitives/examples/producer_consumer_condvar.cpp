/**
 * @file producer_consumer_condvar.cpp
 * @brief Production-grade Thread-Safe Bounded Queue implementing the Classic
 *        Producer-Consumer Pattern using std::condition_variable and std::unique_lock.
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic producer_consumer_condvar.cpp -o prod_cons
 *   ./prod_cons
 */

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

template <typename T>
class BoundedBlockingQueue {
public:
    explicit BoundedBlockingQueue(size_t capacity) 
        : capacity_(capacity), shutdown_(false) {}

    // Push item into queue; blocks if queue is full
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        // Guard against spurious wakeups using while loop predicate
        not_full_cv_.wait(lock, [this]() {
            return queue_.size() < capacity_ || shutdown_;
        });

        if (shutdown_) return;

        queue_.push(std::move(item));

        // Notify one waiting consumer
        not_empty_cv_.notify_one();
    }

    // Pop item from queue; blocks if queue is empty
    // Returns false if queue was shut down and empty
    bool pop(T& out_item) {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait until queue is not empty or shutdown is requested
        not_empty_cv_.wait(lock, [this]() {
            return !queue_.empty() || shutdown_;
        });

        if (queue_.empty() && shutdown_) {
            return false; // Queue drained and shut down
        }

        out_item = std::move(queue_.front());
        queue_.pop();

        // Notify one waiting producer
        not_full_cv_.notify_one();
        return true;
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        // Wake up all sleeping producers and consumers so they can exit gracefully
        not_full_cv_.notify_all();
        not_empty_cv_.notify_all();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    const size_t capacity_;
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
    bool shutdown_;
};

int main() {
    constexpr size_t QUEUE_CAPACITY = 5;
    constexpr int NUM_PRODUCERS = 3;
    constexpr int NUM_CONSUMERS = 3;
    constexpr int ITEMS_PER_PRODUCER = 10;

    BoundedBlockingQueue<int> queue(QUEUE_CAPACITY);
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    std::cout << "Starting Producer-Consumer (Capacity = " << QUEUE_CAPACITY 
              << ", Producers = " << NUM_PRODUCERS 
              << ", Consumers = " << NUM_CONSUMERS << ")...\n\n";

    // Launch Producers
    for (int p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&queue, p]() {
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                int val = p * 100 + i;
                queue.push(val);
                std::cout << "[Producer " << p << "] Produced: " << val << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }

    // Launch Consumers
    for (int c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&queue, c]() {
            int val;
            while (queue.pop(val)) {
                std::cout << "   [Consumer " << c << "] Consumed: " << val << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "   [Consumer " << c << "] Shutting down.\n";
        });
    }

    // Wait for all producers to finish
    for (auto& t : producers) t.join();

    // Signal shutdown to consumers
    queue.shutdown();

    // Wait for consumers to finish draining
    for (auto& t : consumers) t.join();

    std::cout << "\nAll producers and consumers completed successfully!\n";
    return 0;
}
