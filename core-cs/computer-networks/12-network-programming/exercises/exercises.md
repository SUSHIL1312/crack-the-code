# Hands-On Exercises: Network Programming & I/O Multiplexing

---

### Exercise 1: Edge-Triggered Socket Starvation Trap
**Goal**: Identify and resolve the classic starvation bug in Edge-Triggered event demultiplexing.

**Scenario**:
A developer transitions an `epoll` server from Level-Triggered to Edge-Triggered (`EPOLLET`) to reduce context switches.
After deployment, the server intermittently hangs on connections transferring large payloads (e.g. 64 KB HTTP POST bodies).
1. Explain why reading with a single fixed-size `recv(fd, buf, 4096, 0)` causes the connection to freeze indefinitely in Edge-Triggered mode.
2. Write the canonical non-blocking loop pattern in C++ that guarantees all available kernel bytes are drained without hanging or busy-spinning.
3. How does this pattern handle `EAGAIN` / `EWOULDBLOCK` vs `EINTR` vs `ECONNRESET`?

---

### Exercise 2: Implementing Connection Heartbeats & Idle Timeout in an Event Loop
**Goal**: Design a memory-efficient timer mechanism for tracking 100,000 idle TCP connections in an event loop without scanning all sockets every millisecond.

**Scenario**:
You have an event loop managing 100,000 active persistent connections (e.g. WebSocket or gRPC gateway).
If a client goes silent without transmitting an ACK or data for 60 seconds, the server must close the socket to free kernel file descriptors.
1. Why is an $O(N)$ linear iteration across 100,000 connection timestamps on every event loop tick unacceptable?
2. Compare a **Timing Wheel** (Hashed Hierarchical Timer Wheel) vs a **Min-Heap Priority Queue** for managing socket timeout events. What are their time complexities for insertion, cancellation, and tick expiration?
3. How can OS timer descriptors (`timerfd_create` on Linux, `EVFILT_TIMER` on macOS/BSD) be integrated directly into the `epoll` / `kqueue` multiplexer?
