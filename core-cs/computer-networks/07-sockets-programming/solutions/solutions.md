# Solutions: Sockets Programming & Berkeley API Exercises

---

### Solution 2: Non-Blocking `connect_with_timeout` Implementation

Below is a complete, production-grade reference implementation in C++ of non-blocking TCP connection establishment with an explicit timeout.

```cpp
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>

int connect_with_timeout(const std::string& ip, int port, int timeout_ms) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    // Step 1: Set socket to non-blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(fd);
        return -1;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        close(fd);
        return -1;
    }

    // Step 2: Initiate connection (non-blocking)
    int rc = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc < 0) {
        if (errno != EINPROGRESS) {
            // Immediate error
            close(fd);
            return -1;
        }

        // Step 3: Wait for socket to become writable with poll()
        struct pollfd pfd{};
        pfd.fd = fd;
        pfd.events = POLLOUT;

        int poll_ret = poll(&pfd, 1, timeout_ms);
        if (poll_ret <= 0) {
            // Timeout (0) or poll error (< 0)
            close(fd);
            errno = (poll_ret == 0) ? ETIMEDOUT : errno;
            return -1;
        }

        // Step 4: Verify whether connection actually succeeded
        int sock_err = 0;
        socklen_t len = sizeof(sock_err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_err, &len) < 0 || sock_err != 0) {
            close(fd);
            errno = (sock_err != 0) ? sock_err : errno;
            return -1;
        }
    }

    // Step 5: Restore blocking mode
    fcntl(fd, F_SETFL, flags);
    return fd;
}
```
