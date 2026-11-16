# Solutions: Synchronization Primitives

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Counting Semaphore Implementation

```cpp
#include <mutex>
#include <condition_variable>

class CountingSemaphore {
public:
    explicit CountingSemaphore(int initial_count = 0) 
        : count_(initial_count) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        // Blocks until at least one permit/token is available
        cv_.wait(lock, [this]() { return count_ > 0; });
        --count_;
    }

    void release() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
        // Wake one waiting thread
        cv_.notify_one();
    }

    int get_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_;
};
```

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Writer-Preferred Read-Write Lock

```cpp
#include <mutex>
#include <condition_variable>

class WriterPreferredRWLock {
public:
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        // Block readers if a writer is currently writing OR if writers are waiting!
        read_cv_.wait(lock, [this]() {
            return !writer_active_ && waiting_writers_ == 0;
        });
        ++active_readers_;
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        --active_readers_;
        if (active_readers_ == 0 && waiting_writers_ > 0) {
            // Last reader leaving wakes waiting writer
            write_cv_.notify_one();
        }
    }

    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        ++waiting_writers_;
        // Wait until all active readers and active writers finish
        write_cv_.wait(lock, [this]() {
            return !writer_active_ && active_readers_ == 0;
        });
        --waiting_writers_;
        writer_active_ = true;
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        writer_active_ = false;
        if (waiting_writers_ > 0) {
            write_cv_.notify_one(); // Prioritize next writer
        } else {
            read_cv_.notify_all();  // Resume all queued readers
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable read_cv_;
    std::condition_variable write_cv_;
    int active_readers_ = 0;
    int waiting_writers_ = 0;
    bool writer_active_ = false;
};
```

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: The Fair Ticket Spinlock

```cpp
#include <atomic>
#include <thread>

class TicketLock {
public:
    void lock() {
        // 1. Draw a unique sequence number atomically
        const int my_ticket = next_ticket_.fetch_add(1, std::memory_order_relaxed);

        // 2. Spin until our ticket number is called
        while (now_serving_.load(std::memory_order_acquire) != my_ticket) {
            #if defined(__x86_64__)
                __builtin_ia32_pause();
            #elif defined(__aarch64__)
                asm volatile("yield" ::: "memory");
            #endif
        }
    }

    void unlock() {
        // 3. Advance to the next ticket in line
        // Relaxed or release ordering ensures all stores inside critical section are published
        now_serving_.fetch_add(1, std::memory_order_release);
    }

private:
    std::atomic<int> next_ticket_{0};
    std::atomic<int> now_serving_{0};
};
```

### Why It Guarantees FIFO Fairness:
`next_ticket_.fetch_add(1)` establishes an immutable arrival order. A thread that drew ticket 5 will **always** acquire the lock before ticket 6, completely eliminating starvation regardless of thread scheduling or cache line ownership.
