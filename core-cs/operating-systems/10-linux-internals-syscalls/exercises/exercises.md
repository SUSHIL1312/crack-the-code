# Exercises: Linux Internals & System Calls

---

## 🟢 Beginner Level

### Exercise 1.1: Standard Output Redirection via `dup2()`
In Unix shells, typing `./program > output.txt` redirects file descriptor 1 (`stdout`) to `output.txt`.
1. Write a C++ program that:
   - Opens `output.txt` with flags `O_WRONLY | O_CREAT | O_TRUNC` and mode `0644`.
   - Uses `dup2(fd, STDOUT_FILENO)` to duplicate the file descriptor onto `STDOUT_FILENO` (1).
   - Closes the original file descriptor.
   - Executes `std::cout << "This text is redirected to the file!\n";`.
2. Verify that `output.txt` contains the printed text and nothing appears in the terminal.

---

## 🟡 Intermediate Level

### Exercise 2.1: The Self-Pipe Trick for Async Signal Handling
When an event loop server is sleeping in `select()`, `poll()`, or `epoll_wait()`, delivering a signal like `SIGINT` can interrupt the syscall with `EINTR`, but coordinating an orderly shutdown across threads without race conditions is tricky.
1. Implement the **Self-Pipe Trick**:
   - Create a non-blocking pipe using `pipe()` and `fcntl(O_NONBLOCK)`.
   - Register the **read end** of the pipe in your `epoll` or `select` set.
   - In the `SIGINT` / `SIGTERM` signal handler, write a single byte (`'q'`) to the **write end** of the pipe using the async-signal-safe `write()` system call.
   - In the main event loop, reading `'q'` triggers an immediate, thread-safe, and graceful exit!

---

## 🔴 Advanced Level

### Exercise 3.1: Edge-Triggered `epoll` Event Loop
1. Explain why using Edge-Triggered (`EPOLLET`) mode with **blocking sockets** causes permanent server hangs.
2. Write a minimal event-driven TCP server using `epoll` in Edge-Triggered mode:
   - Configure listening socket and accepted client sockets with `O_NONBLOCK`.
   - In the read loop for an active connection, loop `read()` until it returns `-1` with `errno == EAGAIN` or `EWOULDBLOCK`.
   - Handle partial writes gracefully.
