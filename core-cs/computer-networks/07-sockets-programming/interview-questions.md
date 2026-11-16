# Sockets Programming: Top FAANG Interview Questions

### Q1: What is the difference between `SO_REUSEADDR` and `SO_REUSEPORT`? When would you use each in a high-scale backend?
**Answer**:
1. **`SO_REUSEADDR`**:
   - Primarily solves the **`TIME_WAIT` binding issue**.
   - If a server process is restarted while previous sockets are in `TIME_WAIT` (60s), `bind()` fails with `EADDRINUSE`.
   - Setting `SO_REUSEADDR` allows the new process to bind to the port immediately, provided no active listening socket is already bound.
2. **`SO_REUSEPORT` (Linux 3.9+)**:
   - Allows **multiple distinct processes or threads** to bind to the **exact same IP and port simultaneously**!
   - In traditional multi-process servers (e.g. older Nginx), all worker processes shared a single listening socket via `fork()`. When a connection arrived, all workers woke up contending on a mutex or causing the "thundering herd" problem.
   - With `SO_REUSEPORT`, each worker thread has its own private listening socket and its own kernel accept queue. The Linux kernel uses a 4-tuple hash to automatically and evenly distribute incoming client connections across all worker queues with zero lock contention!

---

### Q2: What happens if an application calls `write()` on a TCP socket that has received a `RST` from the remote peer?
**Answer**:
1. **First `write()`**:
   - The kernel notices the connection is reset and sends a `SIGPIPE` signal to the process.
   - If the process ignores `SIGPIPE` (`signal(SIGPIPE, SIG_IGN)`), the `write()` system call returns `-1` with `errno = EPIPE` (*"Broken pipe"*).
2. **Danger of Default Signal Behavior**:
   - By default in POSIX/Linux, the default action for `SIGPIPE` is **immediate process termination (crash)**!
   - Many naive servers crash in production when a client abruptly disconnects or closes their laptop browser while the server is writing a response.
3. **Remediation**:
   - Always ignore `SIGPIPE` on startup: `signal(SIGPIPE, SIG_IGN);`.
   - Or pass `MSG_NOSIGNAL` flag in `send(fd, buf, len, MSG_NOSIGNAL)`.

---

### Q3: Why is `gethostbyname()` deprecated, and why must modern software use `getaddrinfo()`?
**Answer**:
1. **IPv6 Support**:
   - `gethostbyname()` only supports 32-bit IPv4 addresses (`AF_INET`). It has no capability to resolve 128-bit IPv6 (`AAAA`) records.
   - `getaddrinfo()` is completely protocol-agnostic, supporting IPv4, IPv6, or dual-stack lookups.
2. **Thread Safety**:
   - `gethostbyname()` returns a pointer to a **static, internal thread-unsafe buffer** in libc. If two threads resolve hostnames simultaneously, they corrupt each other's return data.
   - `getaddrinfo()` dynamically allocates a new linked list (`struct addrinfo`) for the caller, making it 100% thread-safe.
3. **Integrated Service Resolution**:
   - `getaddrinfo()` resolves both hostname (`"google.com"`) and service name (`"https"`) to port numbers in a single call.
