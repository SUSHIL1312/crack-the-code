#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>

constexpr int NUM_ITERATIONS = 50'000;

void run_server(int server_fd) {
    char buf[256];
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ssize_t bytes = read(server_fd, buf, sizeof(buf));
        if (bytes <= 0) break;

        // Echo response back
        write(server_fd, buf, bytes);
    }
    close(server_fd);
}

void run_client(int client_fd) {
    char req[] = "PING_PAYLOAD_64_BYTES_INTER_PROCESS_COMMUNICATION_BENCHMARK_TEST";
    char resp[256];

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        // Send ping
        if (write(client_fd, req, sizeof(req)) < 0) {
            std::cerr << "Client write failed\n";
            break;
        }

        // Receive echo
        ssize_t bytes = read(client_fd, resp, sizeof(resp));
        if (bytes <= 0) break;
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "[Client] Completed " << NUM_ITERATIONS << " round-trip echo requests\n";
    std::cout << "[Client] Total Time: " << total_ms << " ms\n";
    std::cout << "[Client] Round-Trip Latency per Ping-Pong: " << (total_ms * 1000.0 / NUM_ITERATIONS) << " microseconds\n";
    std::cout << "[Client] Operations per Second: " << (NUM_ITERATIONS / (total_ms / 1000.0)) << " req/sec\n";

    close(client_fd);
}

int main() {
    std::cout << "================ UNIX DOMAIN SOCKET (AF_UNIX) BENCHMARK ================\n";

    int sv[2];
    // socketpair creates an anonymous connected bidirectional stream socket pair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child: Server
        close(sv[0]); // Close client side
        run_server(sv[1]);
        _exit(0);
    } else {
        // Parent: Client
        close(sv[1]); // Close server side
        run_client(sv[0]);
        waitpid(pid, nullptr, 0);
    }

    std::cout << "========================================================================\n";
    return 0;
}
