# Common Mistakes in Sockets Programming

### 1. Crashing on `SIGPIPE` When Clients Disconnect
- **Mistake**: Not handling `SIGPIPE` in a web server or TCP proxy.
- **Reality**: When a mobile client moves out of cellular range or a browser tab is abruptly closed, the client's OS resets the connection. The next server `write()` triggers `SIGPIPE`. By default in Unix/Linux, unhandled `SIGPIPE` causes the server process to terminate instantly!
- **Fix**: Ignore `SIGPIPE` globally at startup:
  ```cpp
  signal(SIGPIPE, SIG_IGN);
  ```

---

### 2. Assuming `send()` Sends All Requested Bytes
- **Mistake**: Writing `send(fd, buffer, 1000000, 0)` and checking only `if (ret < 0)`.
- **Reality**: If the kernel socket send buffer (`SO_SNDBUF`) only has 64 KB of free space, `send()` will write 65,536 bytes and return `65536`. The remaining 934,464 bytes are dropped if the code does not check the return value and loop until all bytes are sent!
- **Fix**: Always wrap `send()` or `write()` in a loop verifying `written += ret` until `written == total`.

---

### 3. Forgetting to Free Memory Allocated by `getaddrinfo()`
- **Mistake**: Calling `getaddrinfo()` to resolve an IP in an API loop and failing to invoke `freeaddrinfo()`.
- **Reality**: `getaddrinfo()` allocates a dynamically linked list of `struct addrinfo` heap structures. Leaking these structures in a high-throughput microservice causes gradual, steady memory growth.
- **Fix**: Wrap `struct addrinfo*` in an RAII custom deleter (`std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>`).

---

### 4. Overlooking Non-Blocking `connect()` Return Semantics
- **Mistake**: Calling `connect()` on a non-blocking socket and assuming return code `-1` means the server is dead.
- **Reality**: In non-blocking mode, `connect()` always returns `-1` with `errno == EINPROGRESS` because the TCP 3-way handshake takes at least one network RTT to complete. You must use `poll()` or `epoll()` to wait for socket writability and check `SO_ERROR`.
