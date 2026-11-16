#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <stdexcept>

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(size_t capacity)
        : capacity_(capacity), closed_(false) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be greater than zero");
        }
    }

    ~BoundedQueue() {
        close();
    }

    // Disable copy/move
    BoundedQueue(const BoundedQueue&) = delete;
    BoundedQueue& operator=(const BoundedQueue&) = delete;
    BoundedQueue(BoundedQueue&&) = delete;
    BoundedQueue& operator=(BoundedQueue&&) = delete;

    // Blocking push: waits if full until space is available or queue closed
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_cv_.wait(lock, [this]() {
            return closed_ || queue_.size() < capacity_;
        });

        if (closed_) return false;

        queue_.push(std::move(item));
        not_empty_cv_.notify_one();
        return true;
    }

    // Timed push: waits up to duration for space
    template <typename Rep, typename Period>
    bool try_push_for(T item, const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        bool acquired = not_full_cv_.wait_for(lock, timeout, [this]() {
            return closed_ || queue_.size() < capacity_;
        });

        if (!acquired || closed_) return false;

        queue_.push(std::move(item));
        not_empty_cv_.notify_one();
        return true;
    }

    // Blocking pop: waits if empty until item arrives or queue closed
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_cv_.wait(lock, [this]() {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty()) return std::nullopt; // Closed and drained

        T item = std::move(queue_.front());
        queue_.pop();
        not_full_cv_.notify_one();
        return item;
    }

    // Timed pop: waits up to duration for item
    template <typename Rep, typename Period>
    std::optional<T> try_pop_for(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        bool acquired = not_empty_cv_.wait_for(lock, timeout, [this]() {
            return closed_ || !queue_.empty();
        });

        if (!acquired || queue_.empty()) return std::nullopt;

        T item = std::move(queue_.front());
        queue_.pop();
        not_full_cv_.notify_one();
        return item;
    }

    // Close the queue: unblocks all waiting threads
    void close() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (closed_) return;
            closed_ = true;
        }
        not_full_cv_.notify_all();
        not_empty_cv_.notify_all();
    }

    bool is_closed() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return closed_;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    size_t capacity() const {
        return capacity_;
    }

private:
    const size_t capacity_;
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
    bool closed_;
};
