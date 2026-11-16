/**
 * @file cache_locality_aos_soa.cpp
 * @brief Benchmark measuring CPU cache locality and memory bandwidth:
 *        Array of Structures (AoS) versus Structure of Arrays (SoA).
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic cache_locality_aos_soa.cpp -o cache_bench
 *   ./cache_bench
 */

#include <iostream>
#include <vector>
#include <chrono>

constexpr size_t NUM_ELEMENTS = 10'000'000;

// 1. Array of Structures (AoS)
// Common in Object-Oriented Programming
struct ParticleAoS {
    float x;
    float y;
    float z;
    float vx;
    float vy;
    float vz;
    int id;
    int flags;
}; // 32 bytes per particle

// 2. Structure of Arrays (SoA)
// Data-Oriented Design (DOD)
struct ParticleSoA {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<float> vx;
    std::vector<float> vy;
    std::vector<float> vz;
    std::vector<int> id;
    std::vector<int> flags;

    explicit ParticleSoA(size_t n) 
        : x(n, 1.0f), y(n, 2.0f), z(n, 3.0f),
          vx(n, 0.5f), vy(n, 0.5f), vz(n, 0.5f),
          id(n, 1), flags(n, 0) {}
};

int main() {
    std::cout << "Benchmarking " << NUM_ELEMENTS << " elements (Updating X position only)...\n";

    // Setup AoS
    std::vector<ParticleAoS> aos_particles(NUM_ELEMENTS);
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        aos_particles[i] = {1.0f, 2.0f, 3.0f, 0.5f, 0.5f, 0.5f, static_cast<int>(i), 0};
    }

    // Benchmark AoS: Loop only touches 'x' and 'vx', but must load entire 32-byte struct into cache!
    auto start_aos = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        aos_particles[i].x += aos_particles[i].vx;
    }
    auto end_aos = std::chrono::high_resolution_clock::now();
    auto aos_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_aos - start_aos).count();

    // Setup SoA
    ParticleSoA soa_particles(NUM_ELEMENTS);

    // Benchmark SoA: Contiguous array of floats. Every single byte fetched into the 64-byte
    // cache line is useful data!
    auto start_soa = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        soa_particles.x[i] += soa_particles.vx[i];
    }
    auto end_soa = std::chrono::high_resolution_clock::now();
    auto soa_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_soa - start_soa).count();

    std::cout << "\n================ CACHE LOCALITY BENCHMARK ================\n";
    std::cout << "Array of Structures (AoS) Time: " << aos_ms << " ms\n";
    std::cout << "Structure of Arrays (SoA) Time: " << soa_ms << " ms\n";
    if (soa_ms > 0) {
        std::cout << "SoA is ~" << static_cast<double>(aos_ms) / soa_ms << "x FASTER than AoS!\n";
    }
    std::cout << "==========================================================\n";

    // Dummy check to prevent dead-code elimination
    if (aos_particles[0].x == 0.0f || soa_particles.x[0] == 0.0f) {
        std::cout << "Dummy\n";
    }

    return 0;
}
