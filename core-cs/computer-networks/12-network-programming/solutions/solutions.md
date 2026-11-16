# Solutions: Network Programming & I/O Multiplexing

---

### Solution 1: Edge-Triggered Socket Starvation Trap

#### 1. Why Single Fixed-Size `recv()` Freezes in ET Mode
In Level-Triggered (LT) mode, `epoll_wait()` / `kevent()` continuously triggers as long as unread data remains in the kernel socket receive buffer.
In **Edge-Triggered (ET)** mode:
- The OS kernel only sends a notification when a **state transition** occurs (from "no data available" to "data available").
- When 64 KB of data arrives:
  1. Kernel notifies `epoll_wait()`.
  2. The application reads only 4096 bytes via a single `recv()`.
  3. $60{,}000\text{ bytes}$ remain unread in the kernel buffer.
  4. The application returns to `epoll_wait()`.
  5. Because no *new* packet arrived to trigger another state transition, `epoll_wait()` **does not fire**!
  6. The client is waiting for an HTTP response; the server is waiting for an `epoll` event. The connection is completely deadlocked.

#### 2. Canonical Non-Blocking Read Loop
```cpp
// Sockets registered with EPOLLET / EV_CLEAR MUST be set to O_NONBLOCK
bool drain_socket_nonblocking(int fd, std::string& accumulated_buffer) {
    char chunk[4096];
    while (true) {
        ssize_t bytes = recv(fd, chunk, sizeof(chunk), 0);
        if (bytes > 0) {
            accumulated_buffer.append(chunk, bytes);
            // Continue looping to drain remaining bytes in kernel buffer
        } else if (bytes == 0) {
            // Orderly shutdown (EOF) - client closed connection (FIN received)
            return false; // Signal caller to close fd
        } else {
            // bytes == -1
            if (errno == EINTR) {
                // Interrupted by OS signal; retry immediately
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // All available data in the kernel buffer has been drained!
                // It is now safe to return to the event loop.
                return true;
            }
            // Real network error (ECONNRESET, ETIMEDOUT, EPIPE, etc.)
            perror("recv error");
            return false; // Close connection
        }
    }
}
```

---

### Solution 2: Implementing Connection Heartbeats & Idle Timeout

#### 1. Why $O(N)$ Linear Iteration Fails
- With 100,000 active sockets, an $O(N)$ scan iterating across an array or hash map of 100,000 timestamps every millisecond or even every 100 ms requires inspecting $100{,}000 \times 10 = 1{,}000{,}000$ objects per second.
- This causes massive CPU cache line eviction, memory bus contention, and degrades event loop throughput.

#### 2. Timing Wheel vs Min-Heap Priority Queue

| Metric / Operation | Min-Heap (`std::priority_queue`) | Timing Wheel (e.g. Hashed / Cascading) |
| :--- | :--- | :--- |
| **Data Structure** | Binary min-heap sorted by expiry timestamp | Circular array of doubly linked lists (slots = seconds) |
| **Add / Reset Timer** | $O(\log N)$ | **$O(1)$** |
| **Cancel Timer** | $O(N)$ without node pointer; $O(\log N)$ with decrease-key | **$O(1)$** (Unlink from doubly linked list) |
| **Tick / Expire** | $O(K \log N)$ where $K$ is number of expired timers | **$O(K)$** (Drain all nodes in current wheel slot) |
| **Memory Overhead** | Compact array | Array of head pointers + 2 pointers per connection |

**Verdict**: High-performance servers (e.g. Netty's `HashedWheelTimer`, Kafka, Linux kernel `timer_list`) use **Hashed Timing Wheels** because every received packet resets a connection's idle timeout. $O(1)$ timer reset amortizes to near-zero CPU overhead.

#### 3. Kernel Timer Integration into Event Multiplexer
Instead of busy-polling time:
- **Linux (`timerfd`)**:
  ```c
  int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  struct itimerspec its = { .it_interval = {1, 0}, .it_value = {1, 0} }; // 1 sec tick
  timerfd_settime(tfd, 0, &its, NULL);
  // Add tfd directly to epoll interest set!
  epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);
  ```
- **macOS / BSD (`kqueue EVFILT_TIMER`)**:
  ```c
  struct kevent timer_ev;
  // Fires an event every 1000 ms
  EV_SET(&timer_ev, 1, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 1000, NULL);
  kevent(kq, &timer_ev, 1, NULL, 0, NULL);
  ```
When the timer fires, `epoll_wait` or `kevent` unblocks with the timer token, allowing the event loop to advance the timing wheel slot and close expired connections seamlessly.
