# FAANG Interview Questions: Network Programming & I/O Multiplexing

---

### Question 1: How does `epoll` achieve $O(1)$ scaling compared to $O(N)$ for `select` and `poll`?
**Target Companies**: Meta, Google, Stripe, Cloudflare, Amazon.

#### Answer:
1. **State Preservation between Syscalls**:
   - `select()` and `poll()` are completely **stateless** across syscall invocations. On every call, user space must serialize and copy the entire set of monitored descriptors into the kernel. The kernel must iterate through every descriptor ($O(N)$), register wait-queues, check status, and copy results back.
   - `epoll` is **stateful**. An `epoll` instance (`struct eventpoll`) is created once via `epoll_create1()`. Descriptors are registered once via `epoll_ctl(EPOLL_CTL_ADD)` and stored in a kernel **Red-Black Tree** ($O(\log N)$ insertion/lookup/deletion).
2. **Callback-Driven Ready Notification**:
   - Rather than scanning all registered sockets on each call to see which ones have data, the kernel attaches an internal callback function (`ep_poll_callback`) to each socket's wait queue.
   - When a network packet arrives and the device driver processes the SoftIRQ, the driver calls `sock_def_readable()`, which invokes `ep_poll_callback()`.
   - This callback directly places the socket's `epitem` onto the ready doubly-linked list (`eventpoll->rdllist`).
3. **Event Retrieval**:
   - When the application calls `epoll_wait()`, the kernel simply checks if `rdllist` is non-empty. If so, it dequeues only the ready items and copies them to user space in $O(K)$ time, where $K$ is the number of **active ready events**, completely independent of the total number of registered connections $N$.

---

### Question 2: What is the "Thundering Herd" problem in multi-threaded network servers, and how is it solved?
**Target Companies**: Google, Nginx, Microsoft, Apple.

#### Answer:
- **The Problem**: Multiple worker threads or processes share a common listening socket or epoll descriptor waiting for incoming TCP connections (`accept()`). When a single new client completes the 3-way handshake, the operating system kernel wakes up **all** listening worker threads simultaneously. Only one thread successfully calls `accept()`; all other threads receive `EAGAIN` or `EWOULDBLOCK` and go back to sleep. This wastes immense CPU time in unnecessary context switches and lock contention.
- **Modern Kernel Solutions**:
  1. **`SO_REUSEPORT` (Linux 3.9+, macOS)**: Allows multiple independent sockets to bind to the exact same IP and port. The kernel maintains an array of listening sockets and uses an internal 4-tuple hash (`hash(src_ip, src_port, dst_ip, dst_port)`) to distribute incoming connections across workers directly at the kernel layer, waking up exactly one worker thread.
  2. **`EPOLLEXCLUSIVE` (Linux 4.5+)**: Added to `epoll_ctl` event flags to instruct the kernel to wake up only one epoll instance when an event occurs on a shared listening socket.
  3. **Application-level Mutex Locking**: Used historically by Nginx (`accept_mutex`), where worker processes take turns holding a shared memory lock before adding the listening socket to their epoll interest set.

---

### Question 3: Explain the difference between the Reactor Pattern and the Proactor Pattern. When would you choose one over the other?
**Target Companies**: Netflix, Amazon (AWS), Uber, Citadel.

#### Answer:

| Dimension | Reactor Pattern | Proactor Pattern |
| :--- | :--- | :--- |
| **Philosophy** | **Readiness-driven**: Informs application when an operation CAN be performed without blocking. | **Completion-driven**: Initiates asynchronous operation and informs application when it HAS finished. |
| **I/O Execution** | Application user-space thread executes the `read()` / `write()` syscall. | Operating system kernel executes the read/write in background. |
| **Buffer Management** | User allocates buffer after socket reports readability. | User pre-allocates buffer before initiating operation; kernel writes directly into it. |
| **OS Primitives** | `epoll` (Linux), `kqueue` (macOS/BSD), `poll`. | Windows IOCP, Linux `io_uring`, POSIX AIO. |
| **Complexity** | State machines must handle partial reads/writes (`EAGAIN`). | Handlers are simpler callbacks, but buffer lifetime management is harder (buffer cannot be freed while async I/O is in-flight). |

---

### Question 4: What is `io_uring` and how does it revolutionize Linux network programming beyond `epoll`?
**Target Companies**: Meta, ByteDance, Datadog.

#### Answer:
- `epoll` still requires making a `read()` or `write()` system call for every event, which incurs user-to-kernel context switch overhead and Spectre/Meltdown page table isolation (KPTI) mitigation penalties ($\approx 1\text{ to }2\ \mu\text{s}$ per syscall).
- **`io_uring`** (Jens Axboe, Linux 5.1+):
  - Uses two lock-free ring buffers in memory mapped shared between user space and kernel space:
    1. **Submission Queue (SQ)**: Application enqueues I/O requests (`read`, `write`, `accept`, `sendmsg`).
    2. **Completion Queue (CQ)**: Kernel enqueues completion status and return values.
  - **Zero-Syscall Mode (`IORING_SETUP_SQPOLL`)**: A dedicated kernel thread continuously polls the Submission Queue. The application can submit thousands of network operations by writing to memory and ringing zero system calls!
  - Supports buffer registration (`io_uring_register_buffers`) to achieve true **Zero-Copy** networking between user space and network device drivers.
