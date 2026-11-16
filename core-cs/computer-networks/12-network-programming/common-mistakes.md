# Common Mistakes: Network Programming & I/O Multiplexing

---

### 1. Registering for `EPOLLOUT` / `EVFILT_WRITE` Permanently
- **The Mistake**: Adding `EPOLLOUT` or `EVFILT_WRITE` to a socket when it is first accepted and leaving it active continuously.
- **Why It Fails**:
  - A TCP socket's kernel transmit buffer is almost always empty (or has available space) under normal network conditions.
  - As a result, the OS multiplexer (`epoll_wait` / `kevent`) **triggers immediately on every single iteration of the event loop**, causing 100% CPU spinning (busy loop) even when the application has zero data to transmit!
- **The Fix**:
  - Only register for `EPOLLOUT` / `EVFILT_WRITE` when a `write()` / `send()` syscall returns `EAGAIN` / `EWOULDBLOCK` (indicating the socket send buffer is full).
  - Once the remaining bytes in the application's outbound queue are completely flushed to the kernel, **immediately deregister** `EPOLLOUT` / `EVFILT_WRITE`.

---

### 2. Forgetting to Set `O_NONBLOCK` on Multiplexed Sockets
- **The Mistake**: Adding standard blocking sockets to `select`, `poll`, `epoll`, or `kqueue`.
- **Why It Fails**:
  - An event multiplexer informs you that a socket is ready, but **that does not guarantee an entire packet can be read without blocking**.
  - For example, if a checksum error causes the kernel to discard a packet after reporting readability, or if fewer bytes than requested arrive, a subsequent blocking `read()` will block the thread indefinitely.
  - If a single-threaded event loop blocks on `read()`, the entire server halts and stops servicing all other 50,000 active client connections!
- **The Fix**: Always set `fcntl(fd, F_SETFL, flags | O_NONBLOCK)` on all sockets immediately upon creation and `accept()`.

---

### 3. Neglecting Partial Writes
- **The Mistake**: Assuming `write()` or `send()` sends the complete buffer:
  ```cpp
  // BUG: Assuming write() sent all 10,000 bytes
  write(fd, big_buf, 10000);
  ```
- **Why It Fails**:
  - In non-blocking mode, the kernel socket send buffer may only have space for 3,420 bytes.
  - `write()` returns `3420` and drops the remaining $6{,}580\text{ bytes}$ on the floor.
  - If the application assumes the write succeeded and moves on, data corruption occurs on the wire.
- **The Fix**: Track the byte offset written. If partial data is accepted, buffer the remainder in a user-space write queue and register for write readiness to flush the rest when buffer space opens up.

---

### 4. Not Handling `SIGPIPE`
- **The Mistake**: Writing to a socket after the remote peer has sent an `RST` without suppressing `SIGPIPE`.
- **Why It Fails**:
  - The default POSIX action for `SIGPIPE` is **immediate process termination** with no core dump!
  - A client closing a browser tab can instantly crash the entire production server process.
- **The Fix**:
  - Ignore `SIGPIPE` at startup: `signal(SIGPIPE, SIG_IGN);`
  - Or use the `MSG_NOSIGNAL` flag on `send()`: `send(fd, buf, len, MSG_NOSIGNAL);`
  - On macOS / BSD, set `SO_NOSIGPIPE`:
    ```c
    int set = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
    ```
