# Interview Questions: Linux Internals & System Calls

---

## Question 1: What is the difference between `select()`, `poll()`, and `epoll()`? Why does `epoll` scale to millions of connections?

* **Difficulty**: Hard
* **Importance**: Critical
* **Category**: Systems I/O & Network Scalability
* **Classification**: Officially documented interview topic (The C10K Problem)
* **Role Relevance**: Systems Engineers, Core Backend Engineers, SRE
* **Prerequisites**: Non-blocking I/O, File Descriptors, Event-driven architecture
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3–5 minute detailed comparison covering data structures, kernel copies, and algorithmic complexity.

### Model Answer
The progression from `select` to `poll` to `epoll` represents the solution to the **C10K problem** (handling 10,000+ concurrent connections):

1. **`select()` ($O(N)$ Overhead, 1024 FD Limit)**:
   - Uses three fixed-size bitmaps (`fd_set`, max 1024 file descriptors).
   - *Problem 1*: On every single call to `select()`, the entire bitmap must be copied from user space to kernel space, and copied back.
   - *Problem 2*: The kernel must perform an $O(N)$ linear loop over all 1024 bits to check readiness.
   - *Problem 3*: When `select()` returns, the application must loop $O(N)$ through all 1024 bits to find which ones fired!
2. **`poll()` ($O(N)$ Overhead, No FD Limit)**:
   - Replaced fixed bitmasks with a dynamic array of `struct pollfd`.
   - Removed the 1024 limit, but still requires copying the entire array to the kernel and performing an $O(N)$ linear scan on every invocation.
3. **`epoll()` ($O(1)$ Event-Driven Scalability)**:
   - **Kernel-Resident Red-Black Tree**: File descriptors are registered once with the kernel using `epoll_ctl()`. The kernel stores them in an internal Red-Black tree ($O(\log N)$ insert/delete). There is **zero copying of the FD list** during event waiting!
   - **Hardware Callback to Ready List (`rdllist`)**: When network packets arrive, the NIC driver interrupt triggers `ep_poll_callback()`, which directly appends the ready socket to a kernel doubly linked list (`rdllist`).
   - **$O(1)$ Dispatch**: `epoll_wait()` does not scan all monitored sockets; it simply returns the contents of the `rdllist`.
   - *Scale*: If 1,000,000 sockets are open and only 5 have active data, `select` scans all 1,000,000 sockets ($O(N)$), while `epoll_wait` instantly returns the 5 ready events in $O(1)$ time!

---

## Question 2: What is the difference between Level-Triggered (LT) and Edge-Triggered (ET) in `epoll`?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Linux Kernel & Network Engineering
* **Classification**: Common industry interview theme
* **Role Relevance**: Backend & Systems Engineers
* **Prerequisites**: `epoll`, Socket buffers, Non-blocking I/O
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute clear comparison highlighting the drain loop.

### Model Answer
- **Level-Triggered (LT - Default)**:
  - `epoll_wait()` acts as a continuous state check: **As long as a socket's buffer has unread data, `epoll_wait()` will keep reporting it as ready**.
  - If 1000 bytes arrive and you read 200 bytes, the next call to `epoll_wait()` will immediately wake up and report the socket again.
  - *Pros*: Simple, safe from data starvation.
  - *Cons*: Extra spurious wakeups if you don't drain the buffer immediately.
- **Edge-Triggered (ET - `EPOLLET`)**:
  - `epoll_wait()` acts as an edge detector: **It reports an event ONLY when the state transitions** (e.g. from empty to ready, or when new packets arrive).
  - If 1000 bytes arrive and you only read 200 bytes, `epoll_wait()` will **NOT** report the socket again until a new network packet arrives! The remaining 800 bytes will be stalled indefinitely.
  - *Requirement*: Sockets MUST be configured as **non-blocking (`O_NONBLOCK`)**, and the application must read in a `while` loop until `read()` returns `-1` with `errno == EAGAIN` (or `EWOULDBLOCK`).
  - *Pros*: Minimizes kernel-to-user context switches; higher performance in high-throughput proxies (like NGINX).

---

## Question 3: Why is calling `printf()` or `malloc()` inside a Unix signal handler strictly prohibited?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Systems Programming & POSIX Safety
* **Classification**: Officially documented interview topic
* **Role Relevance**: Systems, C++, Embedded, Core Software Engineers
* **Prerequisites**: Signals, Mutexes, Async-Signal Safety
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute explanation of reentrancy and deadlock.

### Model Answer
Calling `printf()` or `malloc()` inside a signal handler is strictly prohibited because they are **not async-signal-safe (non-reentrant)**:

1. **Internal Mutex Deadlock**:
   - Both `malloc()` (allocator heap lock) and `printf()` (I/O stream buffer lock) acquire internal global mutexes.
   - An asynchronous signal (like `SIGINT` or `SIGALRM`) can interrupt a thread at **any arbitrary assembly instruction**.
   - If the main thread was in the middle of executing `malloc()` and holding the internal heap lock when the signal arrived, the CPU immediately pauses the main thread and runs the signal handler.
   - If the signal handler calls `malloc()` or `printf()`, it attempts to acquire the **exact same internal lock**.
   - Because the main thread cannot run to release the lock, the process hangs in an **unrecoverable self-deadlock**!
2. **Rule**:
   - Inside signal handlers, you may ONLY invoke **async-signal-safe functions** (such as `write()`, `_exit()`), or set a flag (`volatile sig_atomic_t` or `std::atomic<bool>`) to be handled by the main event loop.
