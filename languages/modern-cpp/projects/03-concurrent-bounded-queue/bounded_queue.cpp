/**
 * bounded_queue.cpp
 * 
 * Production-Quality Concurrent Bounded Queue:
 * - Thread-safe synchronization via std::mutex and std::condition_variable
 * - Zero CPU busy-waiting
 * - Graceful shutdown signaling
 * - Multi-threaded stress test with verification
 */

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <cassert>

template <typename T>
class ConcurrentBoundedQueue {
private:
    std::queue<T> queue_;
    size_t capacity_;
    std::mutex mtx_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
    bool stopped_ = false;

public:
    explicit ConcurrentBoundedQueue(size_t cap) : capacity_(cap) {}

    void push(T item) {
        std::unique_lock<std::mutex> lock(mtx_);
        not_full_cv_.wait(lock, [this]() {
            return queue_.size() < capacity_ || stopped_;
        });
        if (stopped_) return;
        queue_.push(std::move(item));
        not_empty_cv_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        not_empty_cv_.wait(lock, [this]() {
            return !queue_.empty() || stopped_;
        });
        if (queue_.empty() && stopped_) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        not_full_cv_.notify_one();
        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mtx_);
        stopped_ = true;
        not_full_cv_.notify_all();
        not_empty_cv_.notify_all();
    }
};

void run_tests() {
    std::cout << "[TEST] Concurrent Bounded Queue Stress Test...\n";

    ConcurrentBoundedQueue<int> bq(10);
    const int num_producers = 4;
    const int items_per_producer = 250; // 1,000 items total
    std::atomic<int> total_consumed{0};

    // Consumers
    std::vector<std::jthread> consumers;
    for (int i = 0; i < 2; ++i) {
        consumers.emplace_back([&bq, &total_consumed]() {
            int val;
            while (bq.pop(val)) {
                ++total_consumed;
            }
        });
    }

    // Producers
    std::vector<std::jthread> producers;
    for (int i = 0; i < num_producers; ++i) {
        int count = items_per_producer;
        producers.emplace_back([&bq, count]() {
            for (int j = 0; j < count; ++j) {
                bq.push(j);
            }
        });
    }

    // Wait for producers
    producers.clear();
    bq.stop(); // Stop signal
    consumers.clear(); // Wait for consumers to finish draining

    assert(total_consumed.load() == 1000);
    std::cout << "  ✓ 1,000 items successfully produced and consumed concurrently.\n";
}

int main() {
    run_tests();
    std::cout << "✅ Capstone 03 Concurrent Bounded Queue PASSED!\n";
    return 0;
}
