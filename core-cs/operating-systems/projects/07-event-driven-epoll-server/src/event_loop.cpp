#include "event_loop.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

#if defined(__linux__)
#include <sys/epoll.h>

EventLoop::EventLoop() : running_(true) {
    loop_fd_ = epoll_create1(0);
    if (loop_fd_ < 0) {
        perror("epoll_create1");
    }
}

EventLoop::~EventLoop() {
    if (loop_fd_ >= 0) close(loop_fd_);
}

bool EventLoop::add_fd(int fd, uint32_t events, EventCallback callback) {
    struct epoll_event ev{};
    if (events & EVENT_READ)  ev.events |= EPOLLIN;
    if (events & EVENT_WRITE) ev.events |= EPOLLOUT;
    ev.events |= EPOLLET; // Edge-triggered
    ev.data.fd = fd;

    if (epoll_ctl(loop_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll_ctl ADD");
        return false;
    }
    callbacks_[fd] = std::move(callback);
    return true;
}

bool EventLoop::modify_fd(int fd, uint32_t events) {
    struct epoll_event ev{};
    if (events & EVENT_READ)  ev.events |= EPOLLIN;
    if (events & EVENT_WRITE) ev.events |= EPOLLOUT;
    ev.events |= EPOLLET;
    ev.data.fd = fd;

    return epoll_ctl(loop_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool EventLoop::remove_fd(int fd) {
    callbacks_.erase(fd);
    return epoll_ctl(loop_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

int EventLoop::poll_events(int timeout_ms) {
    constexpr int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    int num_ready = epoll_wait(loop_fd_, events, MAX_EVENTS, timeout_ms);
    if (num_ready < 0) {
        if (errno == EINTR) return 0;
        perror("epoll_wait");
        return -1;
    }

    for (int i = 0; i < num_ready; ++i) {
        int fd = events[i].data.fd;
        uint32_t mask = 0;
        if (events[i].events & EPOLLIN)  mask |= EVENT_READ;
        if (events[i].events & EPOLLOUT) mask |= EVENT_WRITE;
        if (events[i].events & (EPOLLERR | EPOLLHUP)) mask |= EVENT_ERROR;

        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
            it->second(fd, mask);
        }
    }
    return num_ready;
}

#elif defined(__APPLE__)
#include <sys/event.h>
#include <sys/time.h>

EventLoop::EventLoop() : running_(true) {
    loop_fd_ = kqueue();
    if (loop_fd_ < 0) {
        perror("kqueue");
    }
}

EventLoop::~EventLoop() {
    if (loop_fd_ >= 0) close(loop_fd_);
}

bool EventLoop::add_fd(int fd, uint32_t events, EventCallback callback) {
    std::vector<struct kevent> changelist;
    if (events & EVENT_READ) {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, nullptr);
        changelist.push_back(ev);
    }
    if (events & EVENT_WRITE) {
        struct kevent ev;
        EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, nullptr);
        changelist.push_back(ev);
    }

    if (!changelist.empty()) {
        if (kevent(loop_fd_, changelist.data(), static_cast<int>(changelist.size()), nullptr, 0, nullptr) < 0) {
            perror("kevent add");
            return false;
        }
    }
    callbacks_[fd] = std::move(callback);
    return true;
}

bool EventLoop::modify_fd(int fd, uint32_t events) {
    // Re-register with EV_ADD | EV_CLEAR
    return add_fd(fd, events, callbacks_[fd]);
}

bool EventLoop::remove_fd(int fd) {
    callbacks_.erase(fd);
    struct kevent ev[2];
    EV_SET(&ev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&ev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    kevent(loop_fd_, ev, 2, nullptr, 0, nullptr);
    return true;
}

int EventLoop::poll_events(int timeout_ms) {
    constexpr int MAX_EVENTS = 64;
    struct kevent eventlist[MAX_EVENTS];

    struct timespec ts{};
    struct timespec* pts = nullptr;
    if (timeout_ms >= 0) {
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1'000'000;
        pts = &ts;
    }

    int num_ready = kevent(loop_fd_, nullptr, 0, eventlist, MAX_EVENTS, pts);
    if (num_ready < 0) {
        if (errno == EINTR) return 0;
        perror("kevent wait");
        return -1;
    }

    for (int i = 0; i < num_ready; ++i) {
        int fd = static_cast<int>(eventlist[i].ident);
        uint32_t mask = 0;
        if (eventlist[i].filter == EVFILT_READ)  mask |= EVENT_READ;
        if (eventlist[i].filter == EVFILT_WRITE) mask |= EVENT_WRITE;
        if (eventlist[i].flags & (EV_EOF | EV_ERROR)) mask |= EVENT_ERROR;

        auto it = callbacks_.find(fd);
        if (it != callbacks_.end()) {
            it->second(fd, mask);
        }
    }
    return num_ready;
}

#endif

void EventLoop::stop() {
    running_ = false;
}
