#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <chrono>
#include <iomanip>

// Pointer chasing struct: each node points to the next node in the array
struct Node {
    Node* next;
    char pad[56]; // Pad to 64 bytes (exact hardware cache line size)
};

// Benchmark cache/memory access latency for a given working set size
double benchmark_latency_ns(size_t size_bytes) {
    size_t count = size_bytes / sizeof(Node);
    if (count < 2) count = 2;

    std::vector<Node> nodes(count);

    // Create a random cycle through all nodes to defeat hardware stream prefetchers
    std::vector<size_t> indices(count);
    std::iota(indices.begin(), indices.end(), 0);

    std::mt19937_64 rng(1337);
    for (size_t i = count - 1; i > 0; --i) {
        std::uniform_int_distribution<size_t> dist(0, i);
        size_t j = dist(rng);
        std::swap(indices[i], indices[j]);
    }

    for (size_t i = 0; i < count - 1; ++i) {
        nodes[indices[i]].next = &nodes[indices[i + 1]];
    }
    nodes[indices[count - 1]].next = &nodes[indices[0]]; // Close cycle

    // Warm up cache
    Node* curr = &nodes[0];
    for (size_t i = 0; i < count * 2; ++i) {
        curr = curr->next;
    }

    // Timed traversal
    constexpr size_t TOTAL_HOPS = 10'000'000;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < TOTAL_HOPS; ++i) {
        curr = curr->next;
    }

    auto end = std::chrono::high_resolution_clock::now();

    // Prevent compiler dead-code elimination
    if (curr == nullptr) std::cerr << "Impossible\n";

    double total_ns = std::chrono::duration<double, std::nano>(end - start).count();
    return total_ns / static_cast<double>(TOTAL_HOPS);
}

int main() {
    std::cout << "================ MEMORY HIERARCHY & CACHE LATENCY BENCHMARK ================\n";
    std::cout << "Measuring random pointer chasing latency across working set sizes.\n";
    std::cout << "Cache line size: 64 bytes (random permutation defeats prefetcher).\n\n";

    struct TestCase {
        size_t size_kb;
        const char* expected_tier;
    };

    std::vector<TestCase> cases = {
        {16,     "L1 Cache (typically <= 64 KB)"},
        {64,     "L1 / L2 Boundary"},
        {256,    "L2 Cache (typically 512 KB - 4 MB)"},
        {1024,   "L2 Cache"},
        {4096,   "L2 / L3 Boundary"},
        {16384,  "L3 Cache (typically 16 MB - 64 MB)"},
        {65536,  "Main RAM (Spills out of L3)"},
        {131072, "Main RAM (High TLB miss rate)"}
    };

    std::cout << std::left
              << std::setw(12) << "Size (KB)"
              << std::setw(18) << "Avg Latency (ns)"
              << "Expected Memory Tier\n";
    std::cout << std::string(65, '-') << "\n";

    for (const auto& tc : cases) {
        double latency = benchmark_latency_ns(tc.size_kb * 1024);
        std::cout << std::left
                  << std::setw(12) << tc.size_kb
                  << std::setw(18) << std::fixed << std::setprecision(2) << latency
                  << tc.expected_tier << "\n";
    }

    std::cout << "============================================================================\n";
    return 0;
}
