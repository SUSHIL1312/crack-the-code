# Solutions: Linux Internals & System Calls

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Standard Output Redirection via `dup2()`

```cpp
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

int main() {
    // 1. Open the target destination file
    int file_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("open failed");
        return 1;
    }

    // 2. dup2 closes STDOUT_FILENO (1) and duplicates file_fd into slot 1
    if (dup2(file_fd, STDOUT_FILENO) < 0) {
        perror("dup2 failed");
        close(file_fd);
        return 1;
    }

    // 3. Close original file descriptor (slot 1 now points to the open file description)
    close(file_fd);

    // 4. Any subsequent write to stdout writes directly to output.txt!
    std::cout << "This text is redirected to the file!\n";
    std::cout << "Process PID: " << getpid() << " written via stdout.\n";

    return 0;
}
```

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: The Self-Pipe Trick

```cpp
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <sys/select.h>

static int g_self_pipe[2];

void async_signal_handler(int signum) {
    (void)signum;
    char token = 'Q';
    // write() is guaranteed async-signal-safe by POSIX
    [[maybe_unused]] auto res = write(g_self_pipe[1], &token, 1);
}

int main() {
    // 1. Create pipe
    if (pipe(g_self_pipe) < 0) return 1;

    // Set write and read ends to non-blocking
    fcntl(g_self_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(g_self_pipe[1], F_SETFL, O_NONBLOCK);

    // 2. Install signal handler
    struct sigaction sa;
    sa.sa_handler = async_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    std::cout << "Event loop running. Press Ctrl+C to test self-pipe shutdown...\n";

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(g_self_pipe[0], &read_fds);

    // 3. Select loop sleeps waiting on pipe or sockets
    int ret = select(g_self_pipe[0] + 1, &read_fds, nullptr, nullptr, nullptr);

    if (ret > 0 && FD_ISSET(g_self_pipe[0], &read_fds)) {
        char token;
        read(g_self_pipe[0], &token, 1);
        std::cout << "\n[Event Loop] Woken by Self-Pipe token '" << token 
                  << "'. Performing clean exit!\n";
    }

    close(g_self_pipe[0]);
    close(g_self_pipe[1]);
    return 0;
}
```

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Edge-Triggered `epoll` Hang Danger
In Edge-Triggered (`EPOLLET`) mode:
- `epoll_wait` only emits an event when the socket transitions from **no data** to **having data**.
- If a socket is **blocking**, and you loop `read()` until end of data, the final `read()` call **blocks permanently** waiting for bytes that haven't arrived yet, freezing the single-threaded event loop!
- If the socket is non-blocking (`O_NONBLOCK`), the final `read()` immediately returns `-1` with `errno == EAGAIN` (or `EWOULDBLOCK`), allowing the loop to cleanly return control to `epoll_wait()`.
