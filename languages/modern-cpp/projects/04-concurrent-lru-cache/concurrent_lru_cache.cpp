/**
 * concurrent_lru_cache.cpp
 * 
 * Production-Quality Concurrent LRU Cache:
 * - O(1) average lookup and eviction
 * - Doubly linked list + std::unordered_map
 * - Concurrency control via std::shared_mutex
 * - Multi-threaded stress test with verification
 */

#include <iostream>
#include <list>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <cassert>

template <typename Key, typename Value>
class ConcurrentLRUCache {
private:
    size_t capacity_;
    mutable std::shared_mutex rw_mtx_;
    std::list<std::pair<Key, Value>> items_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_;

public:
    explicit ConcurrentLRUCache(size_t capacity) : capacity_(capacity) {}

    std::optional<Value> get(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(rw_mtx_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            return std::nullopt;
        }
        // Move accessed item to front (MRU)
        items_.splice(items_.begin(), items_, it->second);
        return it->second->second;
    }

    void put(const Key& key, Value val) {
        std::unique_lock<std::shared_mutex> lock(rw_mtx_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = std::move(val);
            items_.splice(items_.begin(), items_, it->second);
            return;
        }

        // Evict LRU if at capacity
        if (items_.size() >= capacity_) {
            auto lru_key = items_.back().first;
            map_.erase(lru_key);
            items_.pop_back();
        }

        items_.emplace_front(key, std::move(val));
        map_[key] = items_.begin();
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(rw_mtx_);
        return items_.size();
    }
};

void run_tests() {
    std::cout << "[TEST] Concurrent LRU Cache Eviction & Concurrency...\n";

    ConcurrentLRUCache<int, std::string> cache(2);

    cache.put(1, "One");
    cache.put(2, "Two");
    assert(cache.get(1) == "One"); // 1 becomes MRU

    cache.put(3, "Three"); // Evicts 2
    assert(cache.get(2) == std::nullopt);
    assert(cache.get(1) == "One");
    assert(cache.get(3) == "Three");

    // Multithreaded stress test
    ConcurrentLRUCache<int, int> concurrent_cache(100);
    std::vector<std::jthread> threads;

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&concurrent_cache, i]() {
            for (int j = 0; j < 500; ++j) {
                concurrent_cache.put((i * 100) + j, j);
                concurrent_cache.get((i * 100) + j);
            }
        });
    }

    threads.clear(); // Join all threads
    assert(concurrent_cache.size() <= 100);
    std::cout << "  ✓ LRU eviction and thread-safe operations verified.\n";
}

int main() {
    run_tests();
    std::cout << "✅ Capstone 04 Concurrent LRU Cache PASSED!\n";
    return 0;
}
