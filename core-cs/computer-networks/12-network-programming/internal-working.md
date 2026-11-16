# Internal Working: Kernel Readiness Queues & I/O Multiplexing

---

## 1. Deep Dive: Linux `epoll` Kernel Implementation

When an application invokes `epoll_create1()`, the Linux kernel allocates an internal `struct eventpoll` in kernel slab memory:

```c
// Simplified representation from fs/eventpoll.c in Linux Kernel
struct eventpoll {
    spinlock_t lock;
    struct mutex mtx;
    wait_queue_head_t wq;        // Wait queue for epoll_wait callers
    wait_queue_head_t poll_wait; // Used when epoll fd itself is polled
    struct list_head rdllist;    // Doubly linked list of ready items (O(1))
    struct rb_root_cached rbr;   // Red-Black tree root storing all watched fds (O(log N))
    struct epitem *ovflist;      // Overflow list during lock contention
};
```

### The Registration Pipeline (`epoll_ctl` ADD)
1. The kernel allocates an `epitem` structure describing the socket descriptor and event mask.
2. The `epitem` is inserted into the Red-Black Tree (`rbr`), keyed by `(file*, fd)`. Insertion complexity is $O(\log N)$.
3. The kernel registers a callback function `ep_poll_callback` onto the socket’s wait queue (`sk_sleep` / `wait_queue_head_t`).

```
                    +-----------------------+
                    |    eventpoll struct   |
                    +-----------------------+
                     /                     \
                    / (Interest Set)        \ (Ready Set)
                   v                         v
           Red-Black Tree             Doubly-Linked List
             (struct epitem)            (struct epitem)
                 [fd 5]                    [fd 7] <--> [fd 12]
                /      \                     ^
             [fd 3]   [fd 7]                 | (Inserted via
                \        \                   |  ep_poll_callback)
               [fd 4]   [fd 12]              |
                          ^                  |
                          |                  |
               Network Driver RX Interrupt ──+
```

---

## 2. Hardware Interrupt to `epoll_wait` Execution Flow

What happens in physical silicon and OS kernel code when a TCP packet arrives over the wire?

1. **NIC Packet Reception**: The physical network interface card (NIC) writes the incoming packet into a pre-allocated host RAM Ring Buffer via Direct Memory Access (DMA).
2. **Hard IRQ (Interrupt Request)**: The NIC raises a hardware CPU interrupt.
3. **NAPI Polling (SoftIRQ)**: The CPU disables the NIC hard IRQ and executes the network device driver SoftIRQ (`NET_RX_SOFTIRQ`), draining packets via NAPI.
4. **TCP Stack Processing**:
   - Kernel verifies IP checksum, TCP checksum, and sequence numbers.
   - Appends the packet `sk_buff` to the socket's receive queue (`sk_receive_queue`).
5. **Wakeup Callback Triggered**:
   - The TCP stack calls `sock_def_readable(sk)`.
   - This iterates over the socket's wait queue and triggers `ep_poll_callback()`.
6. **Insertion into Ready List**:
   - `ep_poll_callback` adds the corresponding `epitem` directly into `eventpoll->rdllist` ($O(1)$ operation).
   - If a thread was sleeping inside `epoll_wait()`, the kernel wakes up the sleeping thread (`wake_up(&ep->wq)`).
7. **User-Space Return**:
   - `epoll_wait()` extracts items from `rdllist`, copies the event structures to the user-provided array, and returns the number of ready events.

---

## 3. The Mechanics of Edge-Triggered (ET) vs Level-Triggered (LT)

### Level-Triggered (LT) Execution Cycle
1. Packet arrives $\to$ Kernel adds `epitem` to `rdllist`.
2. Application calls `epoll_wait()`. The kernel pops the item from `rdllist` and copies it to user space.
3. If the user-space application only reads 500 bytes of a 2000-byte buffer:
   - At the end of `epoll_wait()`, the kernel re-evaluates the socket. Because `sk_receive_queue` still contains 1500 unread bytes, the kernel **immediately pushes the `epitem` back onto `rdllist`**!
   - Next call to `epoll_wait()` returns immediately.

### Edge-Triggered (ET) Execution Cycle
1. Packet arrives $\to$ Kernel adds `epitem` to `rdllist`.
2. Application calls `epoll_wait()`. The kernel returns the event and removes it from `rdllist`.
3. Even if unread bytes remain in the socket receive buffer, the kernel **does NOT re-add the `epitem` to `rdllist`**.
4. The kernel will **only** re-add the descriptor to `rdllist` when a **brand new state change** occurs (e.g., another new TCP packet arrives from the network).
5. **The Starvation Trap**: If the application did not drain all bytes until `read()` returned `EAGAIN`/`EWOULDBLOCK`, those unread bytes remain locked in kernel memory, and the application will wait forever for an `epoll_wait()` event that will never fire!

---

## 4. Multi-Threaded Thundering Herd & `EPOLLONESHOT`

In a multi-worker event loop architecture where multiple worker threads share an `epoll` descriptor:
- When incoming data arrives on a socket, which thread wakes up?
- If `EPOLLEXCLUSIVE` or `EPOLLONESHOT` is not specified, multiple worker threads may wake up simultaneously to service the same socket descriptor.
- Thread A and Thread B concurrently call `read(fd)`:
  - Thread A reads the packet.
  - Thread B reads `EAGAIN` or reads subsequent packets out of sequence, corrupting protocol state framing.
- **The Solution**: Set the `EPOLLONESHOT` flag on registered sockets:
  - Once an event is reported by `epoll_wait()`, that descriptor is automatically disabled in the interest set.
  - No other worker thread will be woken for that socket while the active thread processes it.
  - Once the active thread finishes processing, it re-arms the socket via `epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev)`.
