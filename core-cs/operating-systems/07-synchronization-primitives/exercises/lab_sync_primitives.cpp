/**
 * ============================================================================
 * LAB 07: Custom Synchronization Primitives (Spinlock, TicketLock, Semaphore)
 * ============================================================================
 * 
 * CANDIDATE INSTRUCTIONS:
 * -----------------------
 * Low-level systems programming requires implementing or understanding synchronization
 * primitives from first principles.
 *
 * In this lab, you will implement three fundamental concurrency primitives:
 *
 * 1. `SpinLock`:
 *    - Implemented using `std::atomic_flag`.
 *    - `lock()`: spins waiting to set the flag using acquire memory order.
 *    - `unlock()`: clears the flag using release memory order.
 *
 * 2. `TicketLock`:
 *    - Fair FIFO spinlock (prevents thread starvation).
 *    - `next_ticket_`: atomics incremented on entry.
 *    - `now_serving_`: atomic serving index.
 *    - Thread takes a ticket via `fetch_add`, then spins until `now_serving_ == my_ticket`.
 *    - `unlock()` increments `now_serving_`.
 *
 * 3. `CountingSemaphore`:
 *    - Implemented using `std::mutex` and `std::condition_variable`.
 *    - Maintains available permits `count_`.
 *    - `acquire()`: waits until `count_ > 0`, then decrements `count_`.
 *    - `release()`: increments `count_` and notifies one waiter.
 *
 * TO RUN TESTS:
 *   make test
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
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // Loop while flag_.test_and_set(std::memory_order_acquire) is true
    }

    void unlock() {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // Clear flag with std::memory_order_release
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
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. uint32_t my_ticket = next_ticket_.fetch_add(1, std::memory_order_relaxed);
        // 2. Loop while now_serving_.load(std::memory_order_acquire) != my_ticket
    }

    void unlock() {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // Increment now_serving_ with std::memory_order_release
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
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. Acquire unique_lock on mtx_
        // 2. cv_.wait(lock, [this]{ return permits_ > 0; })
        // 3. --permits_;
    }

    void release() {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. lock_guard on mtx_
        // 2. ++permits_;
        // 3. cv_.notify_one();
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
// AUTOMATED TEST SUITE (DO NOT MODIFY TEST CODE)
// ============================================================================

void test_spinlock() {
    std::cout << "[TEST 1] Testing SpinLock Mutual Exclusion...\n";
    SpinLock spin;
    spin.lock();
    // A second try to lock or flag check must show locked
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
    std::cout << "  RUNNING TEST SUITE: Lab 07 Synchronization Primitives \n";
    std::cout << "========================================================\n";
    test_spinlock();
    test_ticket_lock();
    test_semaphore();
    std::cout << "\n🎉 ALL TESTS PASSED! Lab 07 Successfully Completed!\n";
    return 0;
}
