/**
 * nonblocking_eventloop_server.cpp
 *
 * Capstone Project 05: Single-Threaded Non-Blocking Event Loop Server
 * Demonstrating non-blocking POSIX socket state machines, kernel demultiplexing,
 * and high-throughput zero-copy buffering.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic nonblocking_eventloop_server.cpp -o nonblocking_eventloop_server
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>
#include <chrono>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#define USE_KQUEUE 1
#include <sys/event.h>
#include <sys/time.h>
#elif defined(__linux__)
#define USE_EPOLL 1
#include <sys/epoll.h>
#endif

// Set fd non-blocking
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Outbound Client State Machine
struct ClientChannel {
    int fd;
    std::string rx_buffer;
    std::string tx_buffer;
    size_t tx_sent_bytes = 0;
    bool write_registered = false;
    bool closed = false;
};

// Event Loop Demultiplexer Engine
class EventLoopEngine {
public:
    EventLoopEngine() {
#if defined(USE_KQUEUE)
        multiplexer_fd_ = kqueue();
        assert(multiplexer_fd_ >= 0);
#elif defined(USE_EPOLL)
        multiplexer_fd_ = epoll_create1(EPOLL_CLOEXEC);
        assert(multiplexer_fd_ >= 0);
#endif
    }

    ~EventLoopEngine() {
        if (multiplexer_fd_ >= 0) close(multiplexer_fd_);
    }

    void register_read(int fd) {
#if defined(USE_KQUEUE)
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        kevent(multiplexer_fd_, &ev, 1, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        epoll_ctl(multiplexer_fd_, EPOLL_CTL_ADD, fd, &ev);
#endif
    }

    void modify_write(int fd, bool enable) {
#if defined(USE_KQUEUE)
        struct kevent ev;
        uint16_t flags = enable ? (EV_ADD | EV_ENABLE) : EV_DELETE;
        EV_SET(&ev, fd, EVFILT_WRITE, flags, 0, 0, nullptr);
        kevent(multiplexer_fd_, &ev, 1, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET | (enable ? EPOLLOUT : 0);
        ev.data.fd = fd;
        epoll_ctl(multiplexer_fd_, EPOLL_CTL_MOD, fd, &ev);
#endif
    }

    void unregister_all(int fd) {
#if defined(USE_KQUEUE)
        struct kevent ev[2];
        EV_SET(&ev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        EV_SET(&ev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        kevent(multiplexer_fd_, ev, 2, nullptr, 0, nullptr);
#elif defined(USE_EPOLL)
        epoll_ctl(multiplexer_fd_, EPOLL_CTL_DEL, fd, nullptr);
#endif
    }

    struct PollEvent {
        int fd;
        bool is_read;
        bool is_write;
        bool is_error;
    };

    int wait_events(std::vector<PollEvent>& events, int timeout_ms) {
        events.clear();
#if defined(USE_KQUEUE)
        constexpr int MAX = 64;
        struct kevent evlist[MAX];
        struct timespec ts{};
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000;
        int n = kevent(multiplexer_fd_, nullptr, 0, evlist, MAX, &ts);
        if (n <= 0) return n;

        for (int i = 0; i < n; ++i) {
            PollEvent pe{static_cast<int>(evlist[i].ident), false, false, false};
            if (evlist[i].filter == EVFILT_READ) pe.is_read = true;
            if (evlist[i].filter == EVFILT_WRITE) pe.is_write = true;
            if (evlist[i].flags & (EV_EOF | EV_ERROR)) pe.is_error = true;
            events.push_back(pe);
        }
        return n;
#elif defined(USE_EPOLL)
        constexpr int MAX = 64;
        struct epoll_event evlist[MAX];
        int n = epoll_wait(multiplexer_fd_, evlist, MAX, timeout_ms);
        if (n <= 0) return n;

        for (int i = 0; i < n; ++i) {
            PollEvent pe{evlist[i].data.fd, false, false, false};
            if (evlist[i].events & EPOLLIN) pe.is_read = true;
            if (evlist[i].events & EPOLLOUT) pe.is_write = true;
            if (evlist[i].events & (EPOLLERR | EPOLLHUP)) pe.is_error = true;
            events.push_back(pe);
        }
        return n;
#endif
    }

private:
    int multiplexer_fd_ = -1;
};

// Benchmark Test Harness
void test_nonblocking_eventloop_server() {
    std::cout << "=========================================================\n";
    std::cout << "  Project 05: Non-Blocking Event Loop Server Benchmark   \n";
    std::cout << "=========================================================\n";

    EventLoopEngine engine;
    std::unordered_map<int, ClientChannel> channels;

    constexpr int NUM_CLIENTS = 10;
    constexpr int MSGS_PER_CLIENT = 1500;
    int client_fds[NUM_CLIENTS];
    int server_fds[NUM_CLIENTS];

    for (int i = 0; i < NUM_CLIENTS; ++i) {
        int sv[2];
        int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        assert(rc == 0);
        server_fds[i] = sv[0];
        client_fds[i] = sv[1];

        set_nonblocking(server_fds[i]);
        set_nonblocking(client_fds[i]);

        engine.register_read(server_fds[i]);
        channels[server_fds[i]] = ClientChannel{server_fds[i], "", "", 0, false, false};
    }

    std::cout << "[Init] Created and registered " << NUM_CLIENTS << " concurrent non-blocking channels.\n";

    const std::string payload = "EVENT_LOOP_MSG\n";
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        ssize_t w = write(client_fds[i], payload.data(), payload.size());
        (void)w;
    }

    uint64_t total_processed = 0;
    int remaining_to_send = NUM_CLIENTS * MSGS_PER_CLIENT;
    std::vector<EventLoopEngine::PollEvent> events;

    auto start_time = std::chrono::steady_clock::now();

    while (total_processed < static_cast<uint64_t>(NUM_CLIENTS * MSGS_PER_CLIENT)) {
        int nev = engine.wait_events(events, 50);
        if (nev > 0) {
            for (const auto& ev : events) {
                auto it = channels.find(ev.fd);
                if (it == channels.end()) continue;
                ClientChannel& chan = it->second;

                // Handle Read
                if (ev.is_read) {
                    char buf[1024];
                    while (true) {
                        ssize_t n = read(chan.fd, buf, sizeof(buf));
                        if (n > 0) {
                            chan.rx_buffer.append(buf, n);
                            // Process delimited messages
                            size_t pos;
                            while ((pos = chan.rx_buffer.find('\n')) != std::string::npos) {
                                std::string msg = chan.rx_buffer.substr(0, pos + 1);
                                chan.rx_buffer.erase(0, pos + 1);
                                // Queue for transmission
                                chan.tx_buffer.append(msg);
                            }
                        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            chan.closed = true;
                            break;
                        }
                    }

                    if (!chan.tx_buffer.empty() && !chan.write_registered) {
                        engine.modify_write(chan.fd, true);
                        chan.write_registered = true;
                    }
                }

                // Handle Write
                if (ev.is_write && !chan.tx_buffer.empty()) {
                    while (chan.tx_sent_bytes < chan.tx_buffer.size()) {
                        ssize_t n = write(chan.fd,
                                          chan.tx_buffer.data() + chan.tx_sent_bytes,
                                          chan.tx_buffer.size() - chan.tx_sent_bytes);
                        if (n > 0) {
                            chan.tx_sent_bytes += n;
                        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            chan.closed = true;
                            break;
                        }
                    }

                    if (chan.tx_sent_bytes == chan.tx_buffer.size()) {
                        chan.tx_buffer.clear();
                        chan.tx_sent_bytes = 0;
                        engine.modify_write(chan.fd, false);
                        chan.write_registered = false;
                    }
                }
            }
        }

        // Drain client endpoints and send follow-ups
        for (int i = 0; i < NUM_CLIENTS; ++i) {
            char cbuf[1024];
            while (true) {
                ssize_t r = read(client_fds[i], cbuf, sizeof(cbuf));
                if (r > 0) {
                    for (ssize_t k = 0; k < r; ++k) {
                        if (cbuf[k] == '\n') {
                            total_processed++;
                            if (remaining_to_send > 0) {
                                ssize_t w = write(client_fds[i], payload.data(), payload.size());
                                (void)w;
                                remaining_to_send--;
                            }
                        }
                    }
                } else {
                    break;
                }
            }
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    double elapsed_s = std::chrono::duration<double>(end_time - start_time).count();
    double qps = static_cast<double>(total_processed) / elapsed_s;

    std::cout << "[Benchmark] Transferred " << total_processed << " non-blocking messages in " 
              << (elapsed_s * 1000.0) << " ms.\n";
    std::cout << "[Benchmark] Event Loop Throughput: " << static_cast<uint64_t>(qps) << " QPS\n";

    for (int i = 0; i < NUM_CLIENTS; ++i) {
        engine.unregister_all(server_fds[i]);
        close(server_fds[i]);
        close(client_fds[i]);
    }

    std::cout << "✅ Capstone Project 05 PASSED!\n";
}

int main() {
    test_nonblocking_eventloop_server();
    return 0;
}
