# Internal Working: Socket Buffers & The Two Backlog Queues

---

## 1. Deep Dive: SYN Queue vs Accept Queue

When a server calls `listen(listen_fd, 128)`:

```
Incoming TCP SYN
       |
       v
+-------------------------------------------------------------+
| SYN Queue (Incomplete Connections)                          |
|   - Capacity: /proc/sys/net/ipv4/tcp_max_syn_backlog        |
|   - Holds sockets in SYN_RCVD state                         |
|   - Server sends SYN-ACK and waits for client ACK           |
+-------------------------------------------------------------+
       |
       | Client sends 3rd Handshake ACK
       v
+-------------------------------------------------------------+
| Accept Queue (Completed Connections)                        |
|   - Capacity: min(backlog argument in listen(), somaxconn)  |
|   - Holds sockets in ESTABLISHED state                      |
|   - Waiting for application to invoke accept()              |
+-------------------------------------------------------------+
       |
       | Application calls accept()
       v
Application receives new client_fd
```

### What happens when the Accept Queue overflows?
If the application is under heavy load (e.g. CPU saturated) and cannot call `accept()` fast enough, the Accept Queue fills up:
- Governed by `/proc/sys/net/ipv4/tcp_abort_on_overflow`:
  - **`0` (Default)**: The kernel **silently drops the incoming 3rd ACK**. The client thinks it is connected, but the server retains the connection in the SYN queue, retransmitting SYN-ACK. When the application finally drains the accept queue, the connection recovers.
  - **`1`**: The kernel immediately returns a `RST` to the client, aborting the connection.

---

## 2. Kernel Socket Buffers (`sk_buff` / `skb`)

Inside the Linux kernel, every network packet is encapsulated in a `struct sk_buff`:

```
+-------------------------------------------------------------+
|                     struct sock                             |
|  sk_rcvbuf: Maximum size of receive buffer (e.g. 212992 B)  |
|  sk_sndbuf: Maximum size of send buffer                     |
|  sk_receive_queue: Doubly linked list of sk_buff            |
|  sk_write_queue:   Doubly linked list of sk_buff            |
+-------------------------------------------------------------+
```

- When `write(sock, buf, len)` is called:
  1. The kernel allocates an `sk_buff` from kernel slab memory (`kmem_cache`).
  2. Copies data from user space `buf` into the `sk_buff` payload using `copy_from_user()`.
  3. Appends the `sk_buff` to `sk_write_queue`.
  4. If the total bytes queued exceed `sk_sndbuf`, `write()` **blocks** (or returns `-1` with `EAGAIN` in non-blocking mode).
- When `read(sock, buf, len)` is called:
  1. If `sk_receive_queue` is empty: `read()` **blocks** waiting on `sk->sk_sleep` wait queue (or returns `EAGAIN` in non-blocking mode).
  2. If data is present: Copies bytes from `sk_buff` to user space `buf` using `copy_to_user()`, frees consumed `sk_buff` pages, and wakes up sender if receive window expanded!
