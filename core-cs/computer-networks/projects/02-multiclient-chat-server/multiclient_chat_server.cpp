/**
 * multiclient_chat_server.cpp
 *
 * Capstone Project 02: Multi-Client Chat Server with Thread-Safe Registry & Broadcast
 * Demonstrating reader-writer concurrency control, broadcast dispatch, and client lifecycle management.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic multiclient_chat_server.cpp -o multiclient_chat_server
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <atomic>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

// Represents an active client connection
struct ClientSession {
    int id;
    int socket_fd;
    std::string username;
};

// Thread-Safe Client Registry with Broadcast Support
class ChatServer {
public:
    ChatServer() : next_client_id_(1) {}

    int register_client(int fd, const std::string& username) {
        std::unique_lock<std::shared_mutex> lock(registry_mutex_);
        int id = next_client_id_++;
        clients_[id] = std::make_shared<ClientSession>(ClientSession{id, fd, username});
        return id;
    }

    void unregister_client(int id) {
        std::unique_lock<std::shared_mutex> lock(registry_mutex_);
        clients_.erase(id);
    }

    size_t active_clients_count() const {
        std::shared_lock<std::shared_mutex> lock(registry_mutex_);
        return clients_.size();
    }

    // Broadcasts a message to all active clients EXCEPT the sender
    void broadcast(int sender_id, const std::string& message) {
        std::vector<std::shared_ptr<ClientSession>> recipients;
        {
            // Hold read lock briefly to snapshot the active client list (avoids deadlock)
            std::shared_lock<std::shared_mutex> lock(registry_mutex_);
            for (const auto& [id, client] : clients_) {
                if (id != sender_id) {
                    recipients.push_back(client);
                }
            }
        }

        // Perform I/O outside of the registry lock
        std::string formatted = message + "\n";
        for (const auto& client : recipients) {
            ssize_t sent = write(client->socket_fd, formatted.data(), formatted.size());
            (void)sent;
        }
    }

private:
    mutable std::shared_mutex registry_mutex_;
    std::unordered_map<int, std::shared_ptr<ClientSession>> clients_;
    std::atomic<int> next_client_id_;
};

// Automated Concurrency Verification Harness
void test_multiclient_chat_broadcast() {
    std::cout << "=========================================================\n";
    std::cout << "  Project 02: Multi-Client Chat Server Broadcast Test    \n";
    std::cout << "=========================================================\n";

    constexpr int NUM_CLIENTS = 4;
    ChatServer server;

    int client_fds[NUM_CLIENTS];
    int server_fds[NUM_CLIENTS];
    int client_ids[NUM_CLIENTS];

    // Initialize socket pairs for all clients
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        int sv[2];
        int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        assert(rc == 0);
        server_fds[i] = sv[0];
        client_fds[i] = sv[1];

        std::string username = "User_" + std::to_string(i + 1);
        client_ids[i] = server.register_client(server_fds[i], username);
    }

    std::cout << "[Registry] Successfully registered " << server.active_clients_count() << " concurrent clients.\n";
    assert(server.active_clients_count() == NUM_CLIENTS);

    // Each client will read incoming broadcasts asynchronously
    std::atomic<int> total_broadcasts_received{0};
    std::vector<std::thread> client_threads;

    // Expected: If each client sends 1 message, every other client receives it.
    // Total broadcasts received across all peers = NUM_CLIENTS * (NUM_CLIENTS - 1)
    constexpr int EXPECTED_BROADCASTS = NUM_CLIENTS * (NUM_CLIENTS - 1);

    for (int i = 0; i < NUM_CLIENTS; ++i) {
        client_threads.emplace_back([i, client_fds, &total_broadcasts_received]() {
            char buf[256];
            int received_count = 0;
            // Count discrete newline-terminated messages even if coalesced
            while (received_count < (NUM_CLIENTS - 1)) {
                ssize_t n = read(client_fds[i], buf, sizeof(buf));
                if (n > 0) {
                    for (ssize_t j = 0; j < n; ++j) {
                        if (buf[j] == '\n') {
                            received_count++;
                            total_broadcasts_received.fetch_add(1);
                        }
                    }
                } else if (n <= 0) {
                    break;
                }
            }
        });
    }

    // Server-side sender threads: each client broadcasts a message through the server
    std::vector<std::thread> sender_threads;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        sender_threads.emplace_back([i, client_ids, &server]() {
            std::string msg = "Hello from Client " + std::to_string(client_ids[i]);
            server.broadcast(client_ids[i], msg);
        });
    }

    for (auto& t : sender_threads) {
        t.join();
    }

    for (auto& t : client_threads) {
        t.join();
    }

    std::cout << "[Broadcast] Total messages received by peers: " 
              << total_broadcasts_received.load() << " / " << EXPECTED_BROADCASTS << "\n";
    assert(total_broadcasts_received.load() == EXPECTED_BROADCASTS);

    // Test unregistering clients
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        server.unregister_client(client_ids[i]);
        close(server_fds[i]);
        close(client_fds[i]);
    }

    assert(server.active_clients_count() == 0);
    std::cout << "[Teardown] All client sessions cleanly deregistered.\n";
    std::cout << "✅ Capstone Project 02 PASSED!\n";
}

int main() {
    test_multiclient_chat_broadcast();
    return 0;
}
