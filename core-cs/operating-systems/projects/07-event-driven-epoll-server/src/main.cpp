#include "event_loop.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <atomic>
#include <cassert>

static bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

void run_client_worker(int client_fd, int client_id, int num_messages, std::atomic<int>& total_echoed) {
    char send_buf[64];
    char recv_buf[64];

    for (int i = 0; i < num_messages; ++i) {
        int len = std::snprintf(send_buf, sizeof(send_buf), "C%d_MSG_%d\n", client_id, i);
        if (write(client_fd, send_buf, len) <= 0) break;

        ssize_t n = read(client_fd, recv_buf, sizeof(recv_buf) - 1);
        if (n <= 0) break;
        recv_buf[n] = '\0';

        if (std::strcmp(send_buf, recv_buf) == 0) {
            total_echoed.fetch_add(1, std::memory_order_relaxed);
        }
    }

    close(client_fd);
}

int main() {
    std::cout << "================ EVENT-DRIVEN NON-BLOCKING SERVER (EPOLL/KQUEUE) ================\n";
    std::cout << "Demonstrating Single-Threaded Reactor Event Loop handling concurrent connections.\n";

    constexpr int NUM_CLIENTS = 8;
    constexpr int MSGS_PER_CLIENT = 2'000;
    constexpr int EXPECTED_TOTAL = NUM_CLIENTS * MSGS_PER_CLIENT;

    EventLoop loop;
    std::atomic<int> total_echoed{0};
    std::vector<int> client_fds;

    // Create concurrent client connections using bidirectional socketpairs
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        int sv[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
            perror("socketpair");
            return 1;
        }

        int server_end = sv[0];
        int client_end = sv[1];

        set_nonblocking(server_end);
        client_fds.push_back(client_end);

        // Register server socket with Reactor EventLoop in Edge-Triggered mode
        loop.add_fd(server_end, EVENT_READ, [&loop](int fd, uint32_t mask) {
            if (mask & EVENT_ERROR) {
                loop.remove_fd(fd);
                close(fd);
                return;
            }

            char buf[512];
            // Edge-triggered drain loop: read until EAGAIN / EWOULDBLOCK
            while (true) {
                ssize_t bytes = read(fd, buf, sizeof(buf));
                if (bytes > 0) {
                    write(fd, buf, bytes); // Echo directly back
                } else if (bytes == 0) {
                    loop.remove_fd(fd);
                    close(fd);
                    break;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break; // Fully drained buffer for this edge trigger
                    }
                    loop.remove_fd(fd);
                    close(fd);
                    break;
                }
            }
        });
    }

    std::cout << "Registered " << NUM_CLIENTS << " concurrent non-blocking socket connections with EventLoop.\n";

    // Spawn client threads
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> clients;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        clients.emplace_back(run_client_worker, client_fds[i], i, MSGS_PER_CLIENT, std::ref(total_echoed));
    }

    // Single-threaded reactor loop demultiplexing all client I/O
    while (total_echoed.load(std::memory_order_relaxed) < EXPECTED_TOTAL) {
        loop.poll_events(10);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    for (auto& t : clients) {
        t.join();
    }

    std::cout << "\nResults:\n";
    std::cout << "  Total Messages Echoed: " << total_echoed.load() << " / " << EXPECTED_TOTAL << "\n";
    assert(total_echoed.load() == EXPECTED_TOTAL);
    std::cout << "  Verification:          PASSED (100% matched responses)\n";
    std::cout << "  Total Execution Time:  " << total_ms << " ms\n";
    std::cout << "  Throughput:            " << (EXPECTED_TOTAL / (total_ms / 1000.0)) << " ops/sec\n";
    std::cout << "==================================================================================\n";

    return 0;
}
