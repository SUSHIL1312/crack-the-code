/**
 * ============================================================================
 * REFERENCE SOLUTION: Lab 07 Synchronization Primitives
 * ============================================================================
 */

#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <cassert>

// 1. SpinLock
class SpinLock {
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Active spin-wait; can add hardware pause instruction
            #if defined(__x86_64__) || defined(_M_X64)
            __builtin_ia32_pause();
            #elif defined(__aarch64__)
            asm volatile("yield");
            #endif
        }
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

    bool is_locked() const {
        return flag_.test(std::memory_order_relaxed);
    }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

// 2. FIFO Fair TicketLock
class TicketLock {
public:
    void lock() {
        uint32_t my_ticket = next_ticket_.fetch_add(1, std::memory_order_relaxed);
        while (now_serving_.load(std::memory_order_acquire) != my_ticket) {
            #if defined(__x86_64__) || defined(_M_X64)
            __builtin_ia32_pause();
            #elif defined(__aarch64__)
            asm volatile("yield");
            #endif
        }
    }

    void unlock() {
        // Increment serving ticket to unblock the next in line
        now_serving_.fetch_add(1, std::memory_order_release);
    }

private:
    std::atomic<uint32_t> next_ticket_{0};
    std::atomic<uint32_t> now_serving_{0};
};

// 3. Counting Semaphore
class CountingSemaphore {
public:
    explicit CountingSemaphore(int initial_permits) : permits_(initial_permits) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]() { return permits_ > 0; });
        --permits_;
    }

    void release() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            ++permits_;
        }
        cv_.notify_one();
    }

    int current_permits() {
        std::lock_guard<std::mutex> lock(mtx_);
        return permits_;
    }

private:
    int permits_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

// ============================================================================
// AUTOMATED TEST SUITE
// ============================================================================

void test_spinlock() {
    std::cout << "[TEST 1] Testing SpinLock Mutual Exclusion...\n";
    SpinLock spin;
    spin.lock();
    bool locked = spin.is_locked();
    assert(locked && "Spinlock must be in locked state after lock()");
    spin.unlock();
    assert(!spin.is_locked() && "Spinlock must be unlocked after unlock()");

    int counter = 0;
    const int num_threads = 8;
    const int incs_per_thread = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < incs_per_thread; ++j) {
                spin.lock();
                ++counter;
                spin.unlock();
            }
        });
    }

    for (auto& t : threads) t.join();
    assert(counter == num_threads * incs_per_thread && "Counter must exactly equal 8000");
    std::cout << "  ✓ Test 1 Passed: SpinLock guaranteed mutual exclusion.\n";
}

void test_ticket_lock() {
    std::cout << "[TEST 2] Testing FIFO Fair TicketLock (8 threads x 1,000 incs)...\n";
    TicketLock ticket;
    int counter = 0;
    const int num_threads = 8;
    const int incs_per_thread = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < incs_per_thread; ++j) {
                ticket.lock();
                ++counter;
                ticket.unlock();
            }
        });
    }

    for (auto& t : threads) t.join();
    assert(counter == num_threads * incs_per_thread && "Counter must exactly equal 8000");
    std::cout << "  ✓ Test 2 Passed: TicketLock guaranteed mutual exclusion.\n";
}

void test_semaphore() {
    std::cout << "[TEST 3] Testing Counting Semaphore (Max 2 concurrent workers)...\n";
    CountingSemaphore sem(2);
    std::atomic<int> active_workers{0};
    std::atomic<bool> violation{false};
    std::vector<std::thread> threads;

    for (int i = 0; i < 6; ++i) {
        threads.emplace_back([&]() {
            sem.acquire();
            int current = ++active_workers;
            if (current > 2) violation = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            --active_workers;
            sem.release();
        });
    }

    for (auto& t : threads) t.join();
    assert(!violation && "More than 2 workers acquired semaphore concurrently!");
    assert(sem.current_permits() == 2 && "All permits must be restored");
    std::cout << "  ✓ Test 3 Passed: Semaphore strictly enforced limit of 2.\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  RUNNING REFERENCE SOLUTION: Lab 07 Sync Primitives    \n";
    std::cout << "========================================================\n";
    test_spinlock();
    test_ticket_lock();
    test_semaphore();
    std::cout << "\n🎉 ALL TESTS PASSED! Lab 07 Solution Verified!\n";
    return 0;
}
