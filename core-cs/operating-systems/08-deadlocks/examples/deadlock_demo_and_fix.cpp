/**
 * @file deadlock_demo_and_fix.cpp
 * @brief Demonstrates a real-world Bank Account Transfer Deadlock (Circular Wait)
 *        and two robust fixes:
 *        1. Address-based Global Lock Ordering
 *        2. Modern C++ std::scoped_lock (C++17)
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic deadlock_demo_and_fix.cpp -o dl_demo
 *   ./dl_demo
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

struct Account {
    int id;
    int balance;
    std::mutex mtx;

    Account(int id_, int balance_) : id(id_), balance(balance_) {}
};

// ======================== 1. BUGGY TRANSFER (DEADLOCK PRONE) ========================
void dangerous_transfer(Account& from, Account& to, int amount) {
    // ANTI-PATTERN: Locking based on parameter order!
    // If Thread 1 calls transfer(AccA, AccB) and Thread 2 calls transfer(AccB, AccA),
    // they acquire locks in opposing order, causing an instant DEADLOCK!
    std::cout << "[Deadlock-Prone] Locking from Account " << from.id << "...\n";
    std::unique_lock<std::mutex> lockA(from.mtx);

    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Widen the race window

    std::cout << "[Deadlock-Prone] Locking to Account " << to.id << "...\n";
    std::unique_lock<std::mutex> lockB(to.mtx);

    from.balance -= amount;
    to.balance += amount;
    std::cout << "Transfer complete: " << amount << " from " << from.id << " to " << to.id << "\n";
}

// ======================== 2. FIX VIA STRICT ADDRESS ORDERING ========================
void safe_transfer_address_ordering(Account& from, Account& to, int amount) {
    if (&from == &to) return; // Self-transfer guard

    // Enforce total ordering based on memory addresses
    if (&from < &to) {
        std::lock_guard<std::mutex> lock1(from.mtx);
        std::lock_guard<std::mutex> lock2(to.mtx);
        from.balance -= amount;
        to.balance += amount;
    } else {
        std::lock_guard<std::mutex> lock1(to.mtx);
        std::lock_guard<std::mutex> lock2(from.mtx);
        from.balance -= amount;
        to.balance += amount;
    }
}

// ======================== 3. FIX VIA MODERN C++ std::scoped_lock ====================
void safe_transfer_scoped_lock(Account& from, Account& to, int amount) {
    if (&from == &to) return;

    // std::scoped_lock atomically acquires multiple mutexes using a deadlock avoidance
    // algorithm (std::lock under the hood), regardless of parameter order!
    std::scoped_lock lock(from.mtx, to.mtx);

    from.balance -= amount;
    to.balance += amount;
}

int main() {
    Account acc1(1, 1000);
    Account acc2(2, 2000);

    std::cout << "--- Testing Deadlock-Free Transfer via std::scoped_lock ---\n";
    std::cout << "Initial Balances: Acc1 = " << acc1.balance << ", Acc2 = " << acc2.balance << "\n\n";

    std::thread t1([&]() {
        for (int i = 0; i < 1000; ++i) {
            safe_transfer_scoped_lock(acc1, acc2, 1);
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 1000; ++i) {
            safe_transfer_scoped_lock(acc2, acc1, 1); // Reverse direction!
        }
    });

    t1.join();
    t2.join();

    std::cout << "Successfully completed 2000 bidirectional transfers without deadlocking!\n";
    std::cout << "Final Balances:   Acc1 = " << acc1.balance << ", Acc2 = " << acc2.balance << "\n";
    std::cout << "---------------------------------------------------------\n";

    return 0;
}
