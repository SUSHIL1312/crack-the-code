# TCP Internals: Top FAANG Interview Questions

### Q1: Why does TCP have a `TIME_WAIT` state, and why is it impossible to safely set `TIME_WAIT` to 0 in production?
**Answer**:
- **Role of TIME_WAIT**: The active closer enters `TIME_WAIT` for $2 \times \text{MSL}$ (Maximum Segment Lifetime, 60 seconds on Linux) after sending the final ACK.
- **Two Critical Purposes**:
  1. **Guaranteeing Reliable Connection Teardown**:
     - If the active closer's final ACK is dropped by the network, the peer retransmits its FIN.
     - If `TIME_WAIT` were set to 0, the active closer would immediately close its socket. Upon receiving the retransmitted FIN, its kernel would reply with a `RST` (connection reset) because the port is closed. The peer would report an error (`Connection reset by peer`) instead of a clean, graceful closure.
  2. **Preventing Old Duplicate Packets from Corrupting Future Connections**:
     - The Internet can buffer, delay, and duplicate packets across asymmetric routes.
     - If `TIME_WAIT = 0`, a client can immediately reuse the same 4-tuple `(client_ip, client_port, server_ip, server_port)`.
     - An old delayed packet from the old connection could arrive at the new connection. If its sequence number falls within the new connection's receive window, the kernel accepts it as valid data, corrupting database writes or file downloads!
- **Safe Solutions to TIME_WAIT Pressure**:
  - Enable HTTP Keep-Alive / connection pooling (reduces connection turnover).
  - Enable Linux `net.ipv4.tcp_tw_reuse = 1` (safely recycles TIME_WAIT sockets if TCP Timestamps prove the packet is strictly newer).
  - Do NOT set `so_linger` to 0 (which sends a brutal `RST` and breaks graceful close).

---

### Q2: What is the difference between Flow Control and Congestion Control in TCP?
**Answer**:
| Dimension | Flow Control | Congestion Control |
| :--- | :--- | :--- |
| **Protected Entity** | The **Receiving Endpoint** (prevents memory buffer overflow). | The **Intermediate Network** (routers, switches, fiber links). |
| **Governing Variable** | Advertised Receive Window (**`rwnd`**). | Congestion Window (**`cwnd`**). |
| **Origin of Feedback** | Explicitly advertised by receiver in the 16-bit Window Size field of TCP header. | Inferred by sender based on packet loss (duplicate ACKs, timeouts) or RTT variations (BBR). |
| **Sender Constraint** | $\text{Inflight Bytes} \le \min(\mathbf{rwnd}, \mathbf{cwnd})$. |

---

### Q3: How do SYN Cookies protect against SYN Flood attacks without keeping state?
**Answer**:
- **The Attack**: Attacker sends millions of `SYN` packets with spoofed IP addresses. A naive server allocates a `struct request_sock` for each SYN in its SYN backlog queue. The queue fills up in milliseconds, starving legitimate users.
- **The SYN Cookie Solution (RFC 4987)**:
  - When the SYN backlog overflows, the server stops allocating memory for new incoming SYNs.
  - Instead, it generates a **cryptographic 32-bit Initial Sequence Number (ISN)**:
    $$\text{ISN} = \text{Timestamp (5b)} \mid \text{MSS Index (3b)} \mid \text{HMAC}(\text{4-tuple}, \text{timestamp}, \text{secret\_key})_{(24b)}$$
  - The server transmits this ISN in the `SYN-ACK` and immediately forgets about the connection!
  - When a legitimate client completes the handshake by sending `ACK (ack_number = ISN + 1)`, the server extracts the ACK number, decrements by 1, and verifies the HMAC using its secret key and the 4-tuple.
  - If the HMAC matches, the connection is instantly established. Attacker SYNs (which never reply with the 3rd ACK) consume **zero server memory**.

---

### Q4: Explain the interaction between Nagle's Algorithm and TCP Delayed ACKs. Why can it cause an unexpected 200 ms latency penalty?
**Answer**:
- **Nagle's Algorithm (`TCP_NODELAY = 0`)**:
  - Designed in 1984 to eliminate the "small-packet problem" (sending 40 bytes of header for 1 byte of Telnet keystroke).
  - Rule: If there is unacknowledged data inflight, the sender buffers all new outgoing small packets until either:
    1. A full MSS-sized packet accumulates, OR
    2. An ACK arrives from the peer.
- **Delayed ACKs (RFC 1122)**:
  - Designed to reduce network traffic by not ACKing every packet immediately.
  - The receiver delays sending an ACK for up to **200 ms** (or until a second full MSS packet arrives, or until the receiver has outgoing data to piggyback).
- **The Deadly 200 ms Stall**:
  1. Client sends a small request header ($< \text{MSS}$).
  2. Client immediately sends a small request body ($< \text{MSS}$).
  3. Nagle's algorithm on the client holds the second packet in memory because packet 1 is unacknowledged.
  4. The server receives packet 1, but waits for packet 2 before sending an application reply.
  5. The server's Delayed ACK timer waits **200 ms** for a second packet before finally sending an ACK.
  6. The client receives the delayed ACK, releases packet 2, and the transaction finishes after a 200 ms stall!
- **Fix**: Disable Nagle's algorithm on latency-sensitive RPC/web servers:
  ```cpp
  int flag = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
  ```
