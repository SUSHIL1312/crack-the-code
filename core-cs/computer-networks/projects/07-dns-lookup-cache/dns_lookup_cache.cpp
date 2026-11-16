/**
 * dns_lookup_cache.cpp
 *
 * Capstone Project 07: High-Performance DNS Lookup & LRU Cache
 * Demonstrating RFC 1035 wire encoding/decoding, TTL expiration, and O(1) LRU eviction.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic dns_lookup_cache.cpp -o dns_lookup_cache
 */

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <memory>
#include <shared_mutex>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <arpa/inet.h>

// DNS Resource Record
struct DnsRecord {
    std::string domain;
    uint16_t type;  // 1 = A, 28 = AAAA, 5 = CNAME
    uint32_t ttl_sec;
    std::string ip_address;
    std::chrono::steady_clock::time_point timestamp;
    bool is_nxdomain = false;

    bool is_expired() const {
        auto now = std::chrono::steady_clock::now();
        auto age_sec = std::chrono::duration_cast<std::chrono::seconds>(now - timestamp).count();
        return age_sec >= ttl_sec;
    }
};

// Thread-Safe O(1) TTL-Aware LRU DNS Cache
class DnsLruCache {
public:
    explicit DnsLruCache(size_t max_capacity) : capacity_(max_capacity) {}

    bool get(const std::string& domain, DnsRecord& out_record) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = map_.find(domain);
        if (it == map_.end()) {
            miss_count_++;
            return false;
        }

        // Check TTL expiration
        if (it->second->is_expired()) {
            lru_list_.erase(it->second);
            map_.erase(it);
            expired_count_++;
            miss_count_++;
            return false;
        }

        // Move to front of LRU list
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
        out_record = *it->second;
        hit_count_++;
        return true;
    }

    void put(const DnsRecord& record) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = map_.find(record.domain);

        if (it != map_.end()) {
            // Update existing entry and move to front
            *it->second = record;
            it->second->timestamp = std::chrono::steady_clock::now();
            lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
            return;
        }

        // Evict LRU tail if at capacity
        if (map_.size() >= capacity_) {
            auto lru_tail = lru_list_.back();
            map_.erase(lru_tail.domain);
            lru_list_.pop_back();
            eviction_count_++;
        }

        DnsRecord r = record;
        r.timestamp = std::chrono::steady_clock::now();
        lru_list_.push_front(r);
        map_[record.domain] = lru_list_.begin();
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.size();
    }

    uint64_t hits() const { return hit_count_; }
    uint64_t misses() const { return miss_count_; }
    uint64_t evictions() const { return eviction_count_; }

private:
    size_t capacity_;
    mutable std::shared_mutex mutex_;
    std::list<DnsRecord> lru_list_;
    std::unordered_map<std::string, std::list<DnsRecord>::iterator> map_;
    uint64_t hit_count_ = 0;
    uint64_t miss_count_ = 0;
    uint64_t eviction_count_ = 0;
    uint64_t expired_count_ = 0;
};

// DNS RFC 1035 Wire Format Helper
class DnsWireEncoder {
public:
    // Encodes a standard domain name (e.g. "google.com") into DNS label wire format: 6google3com0
    static std::vector<uint8_t> encode_domain_name(const std::string& domain) {
        std::vector<uint8_t> encoded;
        size_t start = 0;
        while (start < domain.size()) {
            size_t dot = domain.find('.', start);
            if (dot == std::string::npos) dot = domain.size();

            size_t len = dot - start;
            encoded.push_back(static_cast<uint8_t>(len));
            for (size_t i = start; i < dot; ++i) {
                encoded.push_back(static_cast<uint8_t>(domain[i]));
            }
            start = dot + 1;
        }
        encoded.push_back(0); // Root label null terminator
        return encoded;
    }

    // Decodes wire-format labels back into dotted string
    static std::string decode_domain_name(const uint8_t* wire, size_t offset, size_t total_len) {
        std::string result;
        while (offset < total_len && wire[offset] != 0) {
            uint8_t len = wire[offset++];
            if (offset + len > total_len) break;
            if (!result.empty()) result.push_back('.');
            for (uint8_t i = 0; i < len; ++i) {
                result.push_back(static_cast<char>(wire[offset++]));
            }
        }
        return result;
    }
};

// Automated Verification Test Harness
void test_dns_lookup_and_cache() {
    std::cout << "=========================================================\n";
    std::cout << "  Project 07: High-Performance DNS Lookup & LRU Cache    \n";
    std::cout << "=========================================================\n";

    // Test 1: Wire Format Encoding & Decoding
    {
        std::string test_domain = "www.crackthecode.org";
        auto wire = DnsWireEncoder::encode_domain_name(test_domain);
        std::string decoded = DnsWireEncoder::decode_domain_name(wire.data(), 0, wire.size());
        assert(decoded == test_domain);
        std::cout << "[Test 1] RFC 1035 wire encoding and decoding verified for \"" << test_domain << "\"\n";
    }

    // Test 2: LRU Eviction Under Capacity Constraints
    {
        DnsLruCache cache(3); // Capacity = 3

        cache.put({"google.com", 1, 300, "142.250.190.46", {}, false});
        cache.put({"amazon.com", 1, 300, "205.251.242.103", {}, false});
        cache.put({"microsoft.com", 1, 300, "20.112.52.29", {}, false});
        assert(cache.size() == 3);

        // Access google.com to make it recently used
        DnsRecord rec;
        bool found = cache.get("google.com", rec);
        assert(found);
        assert(rec.ip_address == "142.250.190.46");

        // Insert 4th domain (netflix.com) -> should evict the LRU domain (amazon.com)
        cache.put({"netflix.com", 1, 300, "54.237.226.164", {}, false});
        assert(cache.size() == 3);
        assert(cache.evictions() == 1);

        // Verify amazon.com was evicted (cache miss)
        found = cache.get("amazon.com", rec);
        assert(!found);

        // Verify google.com is still present (cache hit)
        found = cache.get("google.com", rec);
        assert(found);
        std::cout << "[Test 2] LRU eviction verified: amazon.com evicted, google.com retained.\n";
    }

    // Test 3: TTL Expiration and Negative Caching
    {
        DnsLruCache cache(10);

        // Record with 1-second TTL
        cache.put({"fast-expire.com", 1, 1, "1.1.1.1", {}, false});

        // Negative cache entry (NXDOMAIN) with 1-second TTL
        cache.put({"does-not-exist.invalid", 1, 1, "", {}, true});

        DnsRecord rec;
        // Immediate lookup: Cache Hit
        bool found = cache.get("fast-expire.com", rec);
        assert(found);
        assert(rec.ip_address == "1.1.1.1");

        found = cache.get("does-not-exist.invalid", rec);
        assert(found);
        assert(rec.is_nxdomain == true);
        std::cout << "[Test 3] Active TTL and Negative Caching (NXDOMAIN) verified.\n";

        // Wait 1.1 seconds for TTL expiration
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));

        found = cache.get("fast-expire.com", rec);
        assert(!found); // Expired -> Cache Miss!

        found = cache.get("does-not-exist.invalid", rec);
        assert(!found); // Expired -> Cache Miss!
        std::cout << "[Test 3] Expired records cleanly purged upon lookup.\n";
    }

    std::cout << "\n✅ Capstone Project 07 PASSED!\n";
}

int main() {
    test_dns_lookup_and_cache();
    return 0;
}
