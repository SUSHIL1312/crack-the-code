#include "bounded_queue.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cassert>

int main() {
    std::cout << "================ BOUNDED BLOCKING QUEUE VERIFICATION ================\n";

    constexpr size_t CAPACITY = 50;
    BoundedQueue<int> queue(CAPACITY);

    std::cout << "Initialized bounded queue with capacity: " << queue.capacity() << "\n";

    // Test 1: Timed Pop on empty queue
    std::cout << "\n[Test 1] Testing timed pop timeout on empty queue...\n";
    auto start_time = std::chrono::high_resolution_clock::now();
    auto item = queue.try_pop_for(std::chrono::milliseconds(100));
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_time).count();

    assert(!item.has_value());
    std::cout << "Timed pop timed out correctly in " << elapsed << " ms (expected ~100 ms).\n";

    // Test 2: MPMC Stress Test
    std::cout << "\n[Test 2] Multi-Producer Multi-Consumer stress test...\n";
    constexpr int NUM_PRODUCERS = 4;
    constexpr int NUM_CONSUMERS = 4;
    constexpr int ITEMS_PER_PRODUCER = 25'000;
    constexpr int TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    std::atomic<uint64_t> produced_sum{0};
    std::atomic<uint64_t> consumed_sum{0};
    std::atomic<int> consumed_count{0};

    auto bench_start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&queue, &produced_sum, p]() {
            for (int i = 1; i <= ITEMS_PER_PRODUCER; ++i) {
                int val = p * ITEMS_PER_PRODUCER + i;
                queue.push(val);
                produced_sum.fetch_add(val, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&queue, &consumed_sum, &consumed_count]() {
            while (true) {
                auto val = queue.pop();
                if (!val.has_value()) break;
                consumed_sum.fetch_add(*val, std::memory_order_relaxed);
                consumed_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Join all producers
    for (auto& t : producers) t.join();

    // Close queue to wake up and terminate consumers
    queue.close();

    // Join all consumers
    for (auto& t : consumers) t.join();

    auto bench_end = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(bench_end - bench_start).count();

    std::cout << "Successfully transferred " << consumed_count.load() << " items.\n";
    std::cout << "Produced sum: " << produced_sum.load() << "\n";
    std::cout << "Consumed sum: " << consumed_sum.load() << "\n";
    assert(produced_sum.load() == consumed_sum.load());
    std::cout << "Checksum verification: PASSED (100% integrity)!\n";
    std::cout << "Throughput: " << (TOTAL_ITEMS / (total_ms / 1000.0)) << " ops/sec ("
              << total_ms << " ms total)\n";

    std::cout << "======================================================================\n";
    return 0;
}
