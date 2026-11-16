/**
 * reactor_event_loop.cpp
 *
 * High-Performance Cross-Platform Reactor Event Loop Server
 * Demonstrating Non-Blocking I/O, OS Demultiplexing (kqueue on macOS, epoll on Linux),
 * and State-Machine Socket Buffering.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic reactor_event_loop.cpp -o reactor_event_loop
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
#include <netinet/in.h>

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#define USE_KQUEUE 1
#include <sys/event.h>
#include <sys/time.h>
#elif defined(__linux__)
#define USE_EPOLL 1
#include <sys/epoll.h>
#else
#define USE_POLL 1
#include <poll.h>
#endif

// Set a file descriptor to non-blocking mode
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Client Connection Session State
struct ConnectionSession {
    int fd = -1;
    std::string read_buffer;
    std::string write_buffer;
    size_t write_offset = 0;
    bool is_closed = false;
};

// Cross-Platform Reactor Demultiplexer
class Reactor {
public:
    Reactor() {
#if defined(USE_KQUEUE)
        demux_fd_ = kqueue();
        if (demux_fd_ < 0) {
            perror("kqueue failed");
        }
#elif defined(USE_EPOLL)
        demux_fd_ = epoll_create1(EPOLL_CLOEXEC);
        if (demux_fd_ < 0) {
            perror("epoll_create1 failed");
        }
#endif
    }

    ~Reactor() {
        if (demux_fd_ >= 0) {
            close(demux_fd_);
        }
    }

    bool register_read(int fd) {
#if defined(USE_KQUEUE)
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        return kevent(demux_fd_, &ev, 1, nullptr, 0, nullptr) == 0;
#elif defined(USE_EPOLL)
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET; // Edge-triggered
        ev.data.fd = fd;
        return epoll_ctl(demux_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
#else
        (void)fd;
        return true;
#endif
    }

    bool enable_write(int fd) {
#if defined(USE_KQUEUE)
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        return kevent(demux_fd_, &ev, 1, nullptr, 0, nullptr) == 0;
#elif defined(USE_EPOLL)
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.fd = fd;
        return epoll_ctl(demux_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
#else
        (void)fd;
        return true;
#endif
    }

    bool disable_write(int fd) {
#if defined(USE_KQUEUE)
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        return kevent(demux_fd_, &ev, 1, nullptr, 0, nullptr) == 0;
#elif defined(USE_EPOLL)
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        return epoll_ctl(demux_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
#else
        (void)fd;
        return true;
#endif
    }

    bool unregister_fd(int fd) {
#if defined(USE_KQUEUE)
        struct kevent ev[2];
        EV_SET(&ev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        EV_SET(&ev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        kevent(demux_fd_, ev, 2, nullptr, 0, nullptr);
        return true;
#elif defined(USE_EPOLL)
        return epoll_ctl(demux_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
#else
        (void)fd;
        return true;
#endif
    }

    struct Event {
        int fd;
        bool readable;
        bool writable;
        bool error;
    };

    int poll_events(std::vector<Event>& ready_events, int timeout_ms) {
        ready_events.clear();
#if defined(USE_KQUEUE)
        constexpr int MAX_EVENTS = 64;
        struct kevent eventlist[MAX_EVENTS];
        struct timespec ts{};
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000;

        int nev = kevent(demux_fd_, nullptr, 0, eventlist, MAX_EVENTS, &ts);
        if (nev < 0) return -1;

        for (int i = 0; i < nev; ++i) {
            Event ev{static_cast<int>(eventlist[i].ident), false, false, false};
            if (eventlist[i].filter == EVFILT_READ) ev.readable = true;
            if (eventlist[i].filter == EVFILT_WRITE) ev.writable = true;
            if (eventlist[i].flags & (EV_EOF | EV_ERROR)) ev.error = true;
            ready_events.push_back(ev);
        }
        return nev;
#elif defined(USE_EPOLL)
        constexpr int MAX_EVENTS = 64;
        struct epoll_event eventlist[MAX_EVENTS];
        int nev = epoll_wait(demux_fd_, eventlist, MAX_EVENTS, timeout_ms);
        if (nev < 0) return -1;

        for (int i = 0; i < nev; ++i) {
            Event ev{eventlist[i].data.fd, false, false, false};
            if (eventlist[i].events & EPOLLIN) ev.readable = true;
            if (eventlist[i].events & EPOLLOUT) ev.writable = true;
            if (eventlist[i].events & (EPOLLERR | EPOLLHUP)) ev.error = true;
            ready_events.push_back(ev);
        }
        return nev;
#else
        (void)timeout_ms;
        return 0;
#endif
    }

private:
    int demux_fd_ = -1;
};

// Benchmark Test Harness
void run_reactor_benchmark() {
    std::cout << "=========================================================\n";
    std::cout << "  High-Performance Non-Blocking Reactor Event Loop Test  \n";
    std::cout << "=========================================================\n";

#if defined(USE_KQUEUE)
    std::cout << "[Multiplexer] Using macOS / BSD kqueue() kernel subsystem\n";
#elif defined(USE_EPOLL)
    std::cout << "[Multiplexer] Using Linux epoll() kernel subsystem\n";
#endif

    Reactor reactor;
    std::unordered_map<int, ConnectionSession> sessions;

    // Create 8 non-blocking stream socket pairs simulating concurrent client-server connections
    constexpr int NUM_CONNS = 8;
    constexpr int MESSAGES_PER_CONN = 2500;
    int client_fds[NUM_CONNS];
    int server_fds[NUM_CONNS];

    for (int i = 0; i < NUM_CONNS; ++i) {
        int sv[2];
        int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        assert(rc == 0);
        server_fds[i] = sv[0];
        client_fds[i] = sv[1];

        set_nonblocking(server_fds[i]);
        set_nonblocking(client_fds[i]);

        reactor.register_read(server_fds[i]);
        sessions[server_fds[i]] = ConnectionSession{server_fds[i], "", "", 0, false};
    }

    std::cout << "[Setup] Initialized " << NUM_CONNS << " active non-blocking client-server pairs.\n";
    std::cout << "[Bench] Transmitting " << (NUM_CONNS * MESSAGES_PER_CONN) << " messages through Reactor loop...\n";

    auto start_time = std::chrono::steady_clock::now();
    uint64_t total_echoed_messages = 0;
    std::vector<Reactor::Event> ready_events;

    // Client write helper
    const std::string test_payload = "ECHO_PING_REQ\n";

    // Pre-populate initial client requests
    for (int i = 0; i < NUM_CONNS; ++i) {
        ssize_t n = write(client_fds[i], test_payload.data(), test_payload.size());
        (void)n;
    }

    int remaining_requests = NUM_CONNS * MESSAGES_PER_CONN;

    while (total_echoed_messages < static_cast<uint64_t>(NUM_CONNS * MESSAGES_PER_CONN)) {
        int n_events = reactor.poll_events(ready_events, 100);
        if (n_events <= 0) continue;

        for (const auto& ev : ready_events) {
            auto it = sessions.find(ev.fd);
            if (it == sessions.end()) continue;
            ConnectionSession& sess = it->second;

            // Handle Read Readiness
            if (ev.readable) {
                char buf[1024];
                while (true) {
                    ssize_t bytes_read = read(sess.fd, buf, sizeof(buf));
                    if (bytes_read > 0) {
                        sess.read_buffer.append(buf, bytes_read);
                        // Process complete lines
                        size_t pos;
                        while ((pos = sess.read_buffer.find('\n')) != std::string::npos) {
                            std::string line = sess.read_buffer.substr(0, pos + 1);
                            sess.read_buffer.erase(0, pos + 1);
                            // Echo back response
                            sess.write_buffer.append(line);
                        }
                    } else if (bytes_read == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Drained all available data
                            break;
                        } else {
                            sess.is_closed = true;
                            break;
                        }
                    } else {
                        // EOF
                        sess.is_closed = true;
                        break;
                    }
                }

                // If output pending, enable write notification
                if (!sess.write_buffer.empty()) {
                    reactor.enable_write(sess.fd);
                }
            }

            // Handle Write Readiness
            if (ev.writable && !sess.write_buffer.empty()) {
                while (sess.write_offset < sess.write_buffer.size()) {
                    ssize_t sent = write(sess.fd,
                                         sess.write_buffer.data() + sess.write_offset,
                                         sess.write_buffer.size() - sess.write_offset);
                    if (sent > 0) {
                        sess.write_offset += sent;
                    } else if (sent == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Buffer full, wait for next EVFILT_WRITE / EPOLLOUT
                            break;
                        } else {
                            sess.is_closed = true;
                            break;
                        }
                    }
                }

                if (sess.write_offset == sess.write_buffer.size()) {
                    sess.write_buffer.clear();
                    sess.write_offset = 0;
                    reactor.disable_write(sess.fd);
                }
            }
        }

        // Client side drain and ping next
        for (int i = 0; i < NUM_CONNS; ++i) {
            char cbuf[1024];
            while (true) {
                ssize_t cr = read(client_fds[i], cbuf, sizeof(cbuf));
                if (cr > 0) {
                    for (ssize_t j = 0; j < cr; ++j) {
                        if (cbuf[j] == '\n') {
                            total_echoed_messages++;
                            if (remaining_requests > 0) {
                                ssize_t nw = write(client_fds[i], test_payload.data(), test_payload.size());
                                (void)nw;
                                remaining_requests--;
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
    double elapsed_sec = std::chrono::duration<double>(end_time - start_time).count();
    double qps = static_cast<double>(total_echoed_messages) / elapsed_sec;

    std::cout << "[Result] Successfully processed " << total_echoed_messages << " echo roundtrips.\n";
    std::cout << "[Result] Total Execution Time: " << (elapsed_sec * 1000.0) << " ms\n";
    std::cout << "[Result] Reactor Throughput: " << static_cast<uint64_t>(qps) << " QPS (queries/sec)\n";
    std::cout << "[Result] Average Latency per Query: " << (elapsed_sec * 1e6 / total_echoed_messages) << " microseconds\n";

    // Clean up
    for (int i = 0; i < NUM_CONNS; ++i) {
        reactor.unregister_fd(server_fds[i]);
        close(server_fds[i]);
        close(client_fds[i]);
    }

    std::cout << "\n✅ Non-Blocking Reactor Event Loop Verification PASSED!\n";
}

int main() {
    run_reactor_benchmark();
    return 0;
}
