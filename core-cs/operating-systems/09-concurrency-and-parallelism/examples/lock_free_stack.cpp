/**
 * @file lock_free_stack.cpp
 * @brief Complete implementation of a Lock-Free Concurrent Stack (Treiber Stack)
 *        using atomic Compare-And-Swap (compare_exchange_weak).
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -O2 -Wall -Wextra -Wpedantic lock_free_stack.cpp -o lockfree_stack
 *   ./lockfree_stack
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <optional>

template <typename T>
class LockFreeStack {
public:
    struct Node {
        T data;
        Node* next;
        explicit Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    LockFreeStack() : head_(nullptr) {}

    ~LockFreeStack() {
        // Drain remaining nodes sequentially on destruction
        Node* curr = head_.load(std::memory_order_relaxed);
        while (curr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    // Disable copy
    LockFreeStack(const LockFreeStack&) = delete;
    LockFreeStack& operator=(const LockFreeStack&) = delete;

    void push(T val) {
        Node* new_node = new Node(std::move(val));
        
        // Load current head
        new_node->next = head_.load(std::memory_order_relaxed);

        // Atomic Compare-And-Swap loop:
        // If head_ == new_node->next: set head_ = new_node and return true.
        // If another thread pushed in the meantime (head_ != new_node->next):
        // new_node->next is automatically updated with the new head_, and loop retries!
        while (!head_.compare_exchange_weak(
            new_node->next,
            new_node,
            std::memory_order_release, // Synchronizes with pop's acquire
            std::memory_order_relaxed  // Failure order
        )) {
            // Loop automatically retries with updated new_node->next
        }
    }

    std::optional<T> pop() {
        Node* old_head = head_.load(std::memory_order_relaxed);

        // Atomic Compare-And-Swap loop to swing head to old_head->next
        while (old_head && !head_.compare_exchange_weak(
            old_head,
            old_head->next,
            std::memory_order_acquire, // Synchronizes with push's release
            std::memory_order_relaxed
        )) {
            // If CAS fails, old_head is automatically updated with current head_
        }

        if (!old_head) {
            return std::nullopt; // Stack was empty
        }

        T res = std::move(old_head->data);
        delete old_head; // Note: In high-concurrency production, hazard pointers or epoch reclamation is used
        return res;
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) == nullptr;
    }

private:
    std::atomic<Node*> head_;
};

int main() {
    constexpr int NUM_THREADS = 8;
    constexpr int OPS_PER_THREAD = 50'000;

    LockFreeStack<int> stack;

    std::cout << "Pushing " << (NUM_THREADS * OPS_PER_THREAD) 
              << " items across " << NUM_THREADS << " concurrent threads...\n";

    // 1. Concurrent Push
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&stack, i]() {
                for (int j = 0; j < OPS_PER_THREAD; ++j) {
                    stack.push(i * OPS_PER_THREAD + j);
                }
            });
        }
        for (auto& t : threads) t.join();
    }

    std::cout << "Push completed. Now popping concurrently across " << NUM_THREADS << " threads...\n";

    // 2. Concurrent Pop
    std::atomic<int> pop_count{0};
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&stack, &pop_count]() {
                for (int j = 0; j < OPS_PER_THREAD; ++j) {
                    if (stack.pop().has_value()) {
                        pop_count.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
        for (auto& t : threads) t.join();
    }

    std::cout << "\n================ LOCK-FREE STACK RESULTS ================\n";
    std::cout << "Total items successfully popped: " << pop_count.load() << "\n";
    std::cout << "Stack empty? " << (stack.empty() ? "YES" : "NO") << "\n";
    std::cout << "Status: " << (pop_count == NUM_THREADS * OPS_PER_THREAD ? "SUCCESS (Zero drops)" : "FAILED") << "\n";
    std::cout << "=========================================================\n";

    return 0;
}
