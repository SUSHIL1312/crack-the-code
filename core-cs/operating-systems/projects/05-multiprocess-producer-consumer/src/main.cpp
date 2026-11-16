#include "ipc_ring_buffer.hpp"
#include <iostream>
#include <chrono>
#include <sys/wait.h>
#include <cassert>

constexpr uint64_t TEST_MESSAGES = 50'000;

int main() {
    std::cout << "================ MULTI-PROCESS IPC PRODUCER-CONSUMER ================\n";
    std::cout << "Testing zero-copy shared memory IPC synchronized with POSIX semaphores.\n";

    IPCProducerConsumer ipc_channel;

    auto start_time = std::chrono::high_resolution_clock::now();

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child Process: Producer
        for (uint64_t i = 1; i <= TEST_MESSAGES; ++i) {
            IPCMessage msg{};
            msg.seq = i;
            msg.payload_len = std::snprintf(msg.data, sizeof(msg.data), "PAYLOAD_%llu", (unsigned long long)i);
            ipc_channel.produce(msg);
        }
        _exit(0);
    } else {
        // Parent Process: Consumer
        uint64_t expected_seq = 1;
        bool all_valid = true;

        for (uint64_t i = 1; i <= TEST_MESSAGES; ++i) {
            IPCMessage msg = ipc_channel.consume();
            if (msg.seq != expected_seq) {
                std::cerr << "Mismatch! Expected seq " << expected_seq << ", got " << msg.seq << "\n";
                all_valid = false;
            }
            expected_seq++;
        }

        waitpid(pid, nullptr, 0);

        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

        std::cout << "Successfully transferred " << TEST_MESSAGES << " messages across processes.\n";
        std::cout << "Data integrity check: " << (all_valid ? "PASSED (100% in-order, 0 drops)" : "FAILED") << "\n";
        std::cout << "Total elapsed time: " << elapsed_ms << " ms\n";
        std::cout << "Throughput: " << (TEST_MESSAGES / (elapsed_ms / 1000.0)) << " msgs/sec\n";
        std::cout << "Average Latency: " << (elapsed_ms * 1000.0 / TEST_MESSAGES) << " microseconds per message\n";
    }

    std::cout << "======================================================================\n";
    return 0;
}
