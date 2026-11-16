#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

enum EventType : uint32_t {
    EVENT_READ  = 0x01,
    EVENT_WRITE = 0x02,
    EVENT_ERROR = 0x04
};

using EventCallback = std::function<void(int fd, uint32_t events)>;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    // Disable copy/move
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    bool add_fd(int fd, uint32_t events, EventCallback callback);
    bool modify_fd(int fd, uint32_t events);
    bool remove_fd(int fd);

    int poll_events(int timeout_ms);
    void stop();

    bool is_running() const { return running_; }

private:
    int loop_fd_{-1};
    bool running_{false};
    std::unordered_map<int, EventCallback> callbacks_;
};
