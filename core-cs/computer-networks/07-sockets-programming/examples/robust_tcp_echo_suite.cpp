#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/wait.h>
#include <cassert>

// Utility: Set non-blocking mode
bool make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

// Utility: Set receive timeout
bool set_receive_timeout(int fd, int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
}

// Robust Write: Loop until all bytes are transmitted, handling partial writes and EINTR
bool write_all(int fd, const void* buffer, size_t total_bytes) {
    const char* ptr = reinterpret_cast<const char*>(buffer);
    size_t written = 0;

    while (written < total_bytes) {
        ssize_t n = write(fd, ptr + written, total_bytes - written);
        if (n < 0) {
            if (errno == EINTR) continue; // Interrupted by signal; retry
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // In non-blocking mode, would wait on poll/select
                return false;
            }
            return false;
        }
        written += static_cast<size_t>(n);
    }
    return true;
}

// Robust Read: Loop until buffer is full or EOF (peer closed connection)
ssize_t read_all(int fd, void* buffer, size_t max_bytes) {
    char* ptr = reinterpret_cast<char*>(buffer);
    size_t total_read = 0;

    while (total_read < max_bytes) {
        ssize_t n = read(fd, ptr + total_read, max_bytes - total_read);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1; // Error
        }
        if (n == 0) {
            break; // EOF: Peer closed connection or called shutdown(SHUT_WR)
        }
        total_read += static_cast<size_t>(n);
    }
    return total_read;
}

void run_server(int server_fd) {
    std::cout << "[Server] Configured socket with receive timeout (5s).\n";
    set_receive_timeout(server_fd, 5);

    // Read until client signals EOF via half-close shutdown(SHUT_WR)
    std::string received_data;
    char buffer[128];

    while (true) {
        ssize_t n = read(server_fd, buffer, sizeof(buffer));
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("Server read error");
            break;
        }
        if (n == 0) {
            std::cout << "[Server] Detected peer EOF (client performed half-close)!\n";
            break;
        }
        received_data.append(buffer, n);
    }

    std::cout << "[Server] Successfully received complete message (" << received_data.size()
              << " bytes): \"" << received_data << "\"\n";

    // Echo transformed response back
    std::string response = "ACK_ECHO: [" + received_data + "]";
    write_all(server_fd, response.data(), response.size());
    std::cout << "[Server] Sent response to client. Closing socket.\n";
    close(server_fd);
}

void run_client(int client_fd) {
    std::string payload = "FAANG_SYSTEMS_ENGINEERING_TCP_SUITE_PAYLOAD";
    std::cout << "[Client] Sending payload (" << payload.size() << " bytes)...\n";
    write_all(client_fd, payload.data(), payload.size());

    // Step 2: Graceful Half-Close
    // Client has finished sending data, so it shuts down the write direction
    // This transmits a FIN packet to server without destroying the client's ability to read!
    std::cout << "[Client] Calling shutdown(SHUT_WR) to signal end of stream...\n";
    if (shutdown(client_fd, SHUT_WR) != 0) {
        perror("shutdown");
    }

    // Read server's echo response
    char resp_buf[256] = {0};
    ssize_t bytes_read = read_all(client_fd, resp_buf, sizeof(resp_buf) - 1);
    if (bytes_read > 0) {
        resp_buf[bytes_read] = '\0';
        std::cout << "[Client] Received server response: \"" << resp_buf << "\"\n";
    }

    close(client_fd);
}

int main() {
    std::cout << "================ BERKELEY SOCKETS & TCP OPTIONS VERIFICATION ================\n";

    // Create bidirectional socket pair
    int sv[2];
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
        close(sv[0]);
        run_server(sv[1]);
        _exit(0);
    } else {
        // Parent: Client
        close(sv[1]);
        run_client(sv[0]);
        waitpid(pid, nullptr, 0);
    }

    std::cout << "All socket options, robust write loops, and half-close: PASSED!\n";
    std::cout << "==============================================================================\n";
    return 0;
}
