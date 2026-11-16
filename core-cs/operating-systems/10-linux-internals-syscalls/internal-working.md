# Internal Working: System Call ABI, `epoll` Architecture, and Signal Safety

## 1. The System Call Execution Path

When a user program calls `write(1, buf, len)` on x86-64:

```asm
mov rax, 1          ; 1 is the syscall number for SYS_write
mov rdi, 1          ; Argument 1: File descriptor (stdout)
mov rsi, buf        ; Argument 2: Memory pointer to buffer
mov rdx, len        ; Argument 3: Length in bytes
syscall             ; CPU switches to Ring 0, jumps to entry_SYSCALL_64
```

### Kernel Handling:
1. The CPU transitions to Ring 0 and jumps to the address stored in Machine-Specific Register `MSR_LSTAR` (`entry_SYSCALL_64`).
2. The kernel saves user registers into `struct pt_regs` on the kernel stack.
3. The kernel verifies that `RAX < NR_syscalls` and dispatches:
   ```c
   sys_call_table[rax](rdi, rsi, rdx, r10, r8, r9);
   ```
4. The return value is placed back in register `RAX`.
5. **How `errno` is set**: In Linux, the kernel returns a negative error code (e.g. `-EACCES = -13`) directly in `RAX`. The C library (glibc) wrapper checks: If `RAX` is between `-1` and `-4095`, glibc negates the value, stores it in thread-local `errno = -RAX`, and returns `-1` to the application.

---

## 2. Deep Dive: `epoll` Kernel Architecture

When you create an epoll instance via `int epfd = epoll_create1(0);`, the kernel allocates a `struct eventpoll`:

```
                    struct eventpoll
┌────────────────────────────────────────────────────────┐
│  Red-Black Tree (rbr)                                  │
│  [Stores all registered FDs: O(log N) lookup/add/del]  │
├────────────────────────────────────────────────────────┤
│  Ready List (rdllist)                                  │
│  [Doubly linked list of FDs currently ready for I/O]   │
├────────────────────────────────────────────────────────┤
│  Wait Queue (wq)                                       │
│  [Processes sleeping in epoll_wait()]                  │
└────────────────────────────────────────────────────────┘
```

### The Event Notification Pipeline:
1. You register a socket with `epoll_ctl(epfd, EPOLL_CTL_ADD, sock_fd, &ev)`. The socket is inserted into the Red-Black tree.
2. The kernel registers an internal callback function, **`ep_poll_callback()`**, with the socket's wait queue.
3. When network data arrives at the Network Interface Card (NIC), a hardware interrupt fires.
4. The TCP/IP network stack places packets into the socket receive buffer and triggers `ep_poll_callback()`.
5. `ep_poll_callback()` immediately appends the socket to the **`rdllist` (Ready List)** and wakes up the thread blocked in `epoll_wait()`.
6. `epoll_wait()` simply copies the ready events from the `rdllist` to user space in **$O(1)$ time**!

### Level-Triggered (LT) vs Edge-Triggered (ET):
- **Level-Triggered (LT - Default)**:
  - If a socket has 100 bytes ready and you read 40 bytes, the remaining 60 bytes remain in the kernel buffer.
  - The next call to `epoll_wait()` will **notify you again immediately**.
- **Edge-Triggered (ET - `EPOLLET`)**:
  - `epoll_wait()` notifies you **ONLY when a new transition occurs** (e.g. buffer changes from empty to having data).
  - If you only read 40 out of 100 bytes, `epoll_wait()` will **NOT notify you again** until new data arrives!
  - **Mandatory Pattern**: In ET mode, the socket must be **non-blocking (`O_NONBLOCK`)**, and you must read in a `while` loop until `read()` returns `-1` with `errno == EAGAIN` or `EWOULDBLOCK`.

---

## 3. Signals & Async-Signal Safety

When the kernel delivers an asynchronous signal (like `SIGINT` or `SIGTERM`) to a process:
1. The kernel pushes a fake stack frame onto the user stack containing saved registers.
2. The CPU is forced to jump directly to the user-registered signal handler function.
3. The signal handler interrupts whatever line of code was executing at that exact sub-millisecond instant!

### Why You Must NEVER Call `malloc()` or `printf()` in a Signal Handler:
- `malloc()` and `printf()` use internal global mutexes inside glibc.
- Suppose the main application thread is actively executing `malloc()` and has acquired the internal heap lock.
- At that precise instant, a `SIGINT` arrives. The kernel pauses the main thread and runs the signal handler.
- If the signal handler calls `printf("Signal received\n")` or `malloc()`, it attempts to acquire the **exact same internal heap lock**.
- Because the main thread is suspended by the signal, it can never release the lock $\implies$ **Permanent self-deadlock!**
- **POSIX Async-Signal-Safe Functions**: Only a small whitelist of functions (e.g. `write()`, `_exit()`) are guaranteed reentrant and safe inside signal handlers.
- **Modern Idiom**: In the signal handler, only set a `volatile sig_atomic_t` flag or `std::atomic<bool>`, and handle the actual logic in the main event loop.
