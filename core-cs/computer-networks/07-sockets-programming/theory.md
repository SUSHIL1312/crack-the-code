# Theoretical Foundations: Sockets Programming & Berkeley API

---

## 1. The Socket Abstraction

In Unix philosophy, *"Everything is a file"*. A **socket** is a special type of file descriptor representing an endpoint for network communication.
- Created via `int fd = socket(domain, type, protocol)`.
- Can be read and written using standard POSIX file operations (`read()`, `write()`, `close()`), as well as networking-specific calls (`send()`, `recv()`, `setsockopt()`).

### Socket Types:
1. **`SOCK_STREAM` (Stream Socket)**: Sequenced, reliable, two-way connection-based byte streams (typically TCP).
2. **`SOCK_DGRAM` (Datagram Socket)**: Connectionless, unreliable messages of fixed maximum length (typically UDP).
3. **`SOCK_RAW` (Raw Socket)**: Direct access to lower-layer protocols (IP, ICMP) without transport headers (requires root / `CAP_NET_RAW`).

---

## 2. Server vs Client Lifecycle

```
SERVER                                                     CLIENT
socket()                                                   socket()
   |                                                          |
setsockopt(SO_REUSEADDR)                                      |
   |                                                          |
bind() (Assign IP + Port)                                     |
   |                                                          |
listen(backlog) (Enter passive mode)                          |
   |                                                          |
accept() (Blocks until client connects) <==== 3-Way ====> connect()
   |                                       Handshake          |
   |                                                          |
   +-------------> [ Child Socket FD created ] <--------------+
                         |                              |
                      recv() <===== Data Flow ======> send()
                      send() <===== Data Flow ======> recv()
                         |                              |
                      close()                         close()
```

### The Critical Role of `listen(int fd, int backlog)`:
- Transitions the socket from active (connecting) mode to passive (listening) mode.
- In modern Linux, the kernel maintains **two queues**:
  1. **SYN Queue (Incomplete Connection Queue)**: Holds sockets in the `SYN_RCVD` state (waiting for client's 3rd ACK).
  2. **Accept Queue (Completed Connection Queue)**: Holds sockets in the `ESTABLISHED` state that have completed the 3-way handshake and are waiting for the application to call `accept()`.
  - The `backlog` argument sets the maximum length of the Accept Queue (capped by `/proc/sys/net/core/somaxconn`).

---

## 3. Essential Socket Options (`setsockopt`)

```cpp
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
```

| Option | Level | Purpose | Recommended Setting |
| :--- | :--- | :--- | :--- |
| **`SO_REUSEADDR`** | `SOL_SOCKET` | Allows binding to a port currently in the `TIME_WAIT` state. | **Always set** on server listening sockets before `bind()`. |
| **`SO_REUSEPORT`** | `SOL_SOCKET` | Allows multiple independent processes/threads to bind to the **exact same port**. Kernel automatically load-balances incoming connections across all listening sockets! | Set on multi-worker servers (e.g. Nginx, Envoy). |
| **`TCP_NODELAY`** | `IPPROTO_TCP` | Disables Nagle's algorithm. Sends small packets immediately without waiting 200ms for ACK. | Set on RPCs, financial trading, gaming, and HTTP APIs. |
| **`SO_KEEPALIVE`** | `SOL_SOCKET` | Periodically sends TCP keepalive probe packets on idle connections to detect silent peer crashes or dead NAT tunnels. | Set on long-lived connections (database connection pools, gRPC). |
| **`SO_RCVTIMEO` / `SO_SNDTIMEO`** | `SOL_SOCKET` | Sets a timeout on blocking `recv()` / `send()` syscalls. | Set to prevent threads from hanging forever on dead clients. |

---

## 4. Half-Close Semantics (`shutdown` vs `close`)

- **`close(fd)`**:
  - Decrements the file descriptor reference count across processes.
  - When the reference count reaches 0, tears down **both read and write directions** and deallocates the socket.
- **`shutdown(fd, how)`**:
  - Shuts down a specific half of the full-duplex connection without destroying the socket descriptor:
    - `SHUT_WR`: Sends a `FIN` packet to peer. Application can no longer write, but **can continue to read incoming data** from peer!
    - `SHUT_RD`: Disables further receive operations; any data in receive buffer is discarded.
    - `SHUT_RDWR`: Shuts down both directions.
  - *Typical Use Case*: A client finishes uploading a 100 MB file, calls `shutdown(fd, SHUT_WR)` to signal EOF to the server, and waits to read the server's HTTP 200 response.
