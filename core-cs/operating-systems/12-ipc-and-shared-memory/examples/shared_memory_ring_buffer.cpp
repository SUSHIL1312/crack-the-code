#include <iostream>
#include <atomic>
#include <chrono>
#include <cstring>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

// Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer in Shared Memory
constexpr size_t RING_CAPACITY = 1024;
constexpr uint64_t TOTAL_MESSAGES = 500'000;

struct Message {
    uint64_t seq;
    uint64_t timestamp_ns;
    char payload[32];
};

struct alignas(64) SharedRingBuffer {
    // Separate cache lines (alignas(64)) to avoid false sharing between producer and consumer
    alignas(64) std::atomic<size_t> head{0}; // Written by Producer
    alignas(64) std::atomic<size_t> tail{0}; // Written by Consumer
    Message slots[RING_CAPACITY];
};

void run_producer(SharedRingBuffer* rb) {
    auto start = std::chrono::high_resolution_clock::now();

    for (uint64_t i = 1; i <= TOTAL_MESSAGES; ++i) {
        size_t current_head = rb->head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % RING_CAPACITY;

        // Spin-wait if buffer is full
        while (next_head == rb->tail.load(std::memory_order_acquire)) {
#if defined(__x86_64__)
            __builtin_ia32_pause();
#elif defined(__aarch64__)
            asm volatile("yield");
#endif
        }

        // Write message data directly to shared RAM
        rb->slots[current_head].seq = i;
        rb->slots[current_head].timestamp_ns = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count());
        std::snprintf(rb->slots[current_head].payload, sizeof(rb->slots[current_head].payload), "Msg-%llu", (unsigned long long)i);

        // Publish new head with release semantics
        rb->head.store(next_head, std::memory_order_release);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "[Producer] Sent " << TOTAL_MESSAGES << " messages in " << total_ms << " ms ("
              << (TOTAL_MESSAGES / total_ms * 1000.0) / 1e6 << " million msgs/sec)\n";
}

void run_consumer(SharedRingBuffer* rb) {
    uint64_t received = 0;
    uint64_t last_seq = 0;
    bool sequence_correct = true;

    auto start = std::chrono::high_resolution_clock::now();

    while (received < TOTAL_MESSAGES) {
        size_t current_tail = rb->tail.load(std::memory_order_relaxed);

        // Spin-wait if buffer is empty
        while (current_tail == rb->head.load(std::memory_order_acquire)) {
#if defined(__x86_64__)
            __builtin_ia32_pause();
#elif defined(__aarch64__)
            asm volatile("yield");
#endif
        }

        // Read message
        Message msg = rb->slots[current_tail];
        received++;

        if (msg.seq != last_seq + 1) {
            sequence_correct = false;
        }
        last_seq = msg.seq;

        // Advance tail with release semantics
        size_t next_tail = (current_tail + 1) % RING_CAPACITY;
        rb->tail.store(next_tail, std::memory_order_release);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "[Consumer] Received " << received << " messages in " << total_ms << " ms\n";
    std::cout << "[Consumer] Ordering verification: " << (sequence_correct ? "PASSED (100% in-order)" : "FAILED") << "\n";
    std::cout << "[Consumer] Average Latency per message: " << (total_ms * 1000.0 / received) << " microseconds\n";
}

int main() {
    std::cout << "================ ZERO-COPY SHARED MEMORY SPSC BENCHMARK ================\n";

    // Allocate anonymous shared memory (MAP_SHARED | MAP_ANONYMOUS)
    void* ptr = mmap(nullptr, sizeof(SharedRingBuffer),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANON, -1, 0);

    if (ptr == MAP_FAILED) {
        std::cerr << "mmap failed\n";
        return 1;
    }

    // Placement-new to initialize atomics
    SharedRingBuffer* ring_buffer = new (ptr) SharedRingBuffer();

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "fork failed\n";
        return 1;
    }

    if (pid == 0) {
        // Child process = Producer
        run_producer(ring_buffer);
        _exit(0);
    } else {
        // Parent process = Consumer
        run_consumer(ring_buffer);
        waitpid(pid, nullptr, 0);
    }

    ring_buffer->~SharedRingBuffer();
    munmap(ptr, sizeof(SharedRingBuffer));

    std::cout << "========================================================================\n";
    return 0;
}
