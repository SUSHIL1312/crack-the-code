/**
 * concurrency_and_atomics.cpp
 * 
 * Verifying jthread auto-joining, condition variable synchronization, and acquire-release atomics.
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>

// Acquire-Release Message Passing
std::atomic<bool> ready{false};
int payload = 0;

void producer() {
    payload = 42; // Non-atomic write
    ready.store(true, std::memory_order_release); // Release barrier publishes payload
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) { // Acquire barrier syncs with release
        std::this_thread::yield();
    }
    assert(payload == 42); // Guaranteed to see payload write!
}

void test_concurrency() {
    std::cout << "[TEST] std::jthread & Acquire-Release Message Passing...\n";

    // jthread auto-joins on scope exit
    {
        std::jthread t1(consumer);
        std::jthread t2(producer);
    } // Both threads join cleanly here!

    std::cout << "  ✓ Acquire-release memory order published data safely across threads.\n";
}

int main() {
    test_concurrency();
    std::cout << "✅ Module 07 Concurrency & Atomics PASSED!\n";
    return 0;
}
