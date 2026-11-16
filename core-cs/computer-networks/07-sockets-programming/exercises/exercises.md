# Hands-On Exercises: Sockets Programming & Berkeley API

---

### Exercise 1: Multi-Threaded TCP Server with `SO_REUSEPORT`
**Goal**: Build a multi-worker TCP echo server in C++ where multiple worker threads each create and bind their own independent listening socket to the same port using `SO_REUSEPORT`.

**Requirements**:
1. Spawn 4 worker threads.
2. In each thread, create a socket, enable `SO_REUSEADDR` and `SO_REUSEPORT`, bind to the same port (e.g. 9000), and call `listen()`.
3. In each thread, enter an `accept()` loop and process incoming connections independently.
4. Verify that the Linux kernel distributes incoming connections evenly across all 4 worker threads without needing an application-level mutex or dispatcher thread!

---

### Exercise 2: Implementing Non-Blocking `connect()` with Timeout
**Goal**: Implement a robust non-blocking connection function `connect_with_timeout(const char* host, int port, int timeout_ms)`.

**Requirements**:
1. Put the socket into non-blocking mode via `fcntl(fd, F_SETFL, O_NONBLOCK)`.
2. Call `connect()`. Since the connection is non-blocking, `connect()` immediately returns `-1` with `errno == EINPROGRESS`.
3. Use `poll()` or `select()` to wait for the socket descriptor to become writable within `timeout_ms`.
4. Inspect the asynchronous connection result using `getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)`. If `error == 0`, connection succeeded; otherwise, report the error.
5. Restore the socket to blocking mode before returning.

---

### Exercise 3: Handling `EINTR` & Partial Reads/Writes
**Goal**: Write a production utility library providing `read_n_bytes(int fd, void* buf, size_t n)` and `write_n_bytes(int fd, const void* buf, size_t n)`.

**Requirements**:
1. Handle `EINTR` (system call interrupted by signal handler) by restarting the system call.
2. Handle partial writes where `write()` transfers fewer bytes than requested.
3. Handle partial reads where `read()` returns a fraction of the requested bytes.
4. Detect EOF when `read()` returns 0.
