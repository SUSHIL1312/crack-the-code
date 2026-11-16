# Theoretical Foundations: UDP, Head-of-Line Blocking, and QUIC

---

## 1. Why HTTP/2 Suffered from Transport Head-of-Line Blocking

HTTP/1.1 suffered from **Application-Layer Head-of-Line Blocking** because a client could only execute one request-response cycle at a time over a single TCP connection. Browsers worked around this by opening up to 6 parallel TCP connections per domain.

HTTP/2 introduced **Binary Stream Multiplexing**:
- Interleaves hundreds of concurrent logical request/response streams over a **single shared TCP connection**.
- This solved application-layer blocking, but exposed a deeper problem: **Transport-Layer Head-of-Line Blocking**:
  - The OS kernel TCP stack views the connection as a single, contiguous byte stream. It has **no concept of HTTP/2 streams**.
  - If a single IP packet containing 1,460 bytes of Stream 1 is dropped on the network, the receiver's TCP stack **refuses to deliver any subsequent bytes to the application** (even if packets for Streams 2, 3, and 4 have already arrived in kernel RAM).
  - All 100 concurrent HTTP/2 streams freeze until the sender retransmits the missing packet for Stream 1!
  - On lossy networks (e.g. 2% packet loss on mobile cellular), HTTP/2 was often **slower than HTTP/1.1 with 6 separate connections**!

---

## 2. QUIC: A Modern Transport Protocol over UDP (RFC 9000)

QUIC solves the architectural flaws of TCP while running on top of UDP in user space.

### 1. Independent Logical Streams
- Within a single QUIC connection, hundreds of lightweight, bidirectional or unidirectional streams can be created.
- Each stream has its own independent stream offset and flow control.
- If a packet carrying data for Stream 1 is dropped:
  - Stream 1 pauses waiting for retransmission.
  - Streams 2, 3, and 4 **continue delivering data to the application with zero interruption**!

### 2. Zero-RTT Connection Establishment (0-RTT Handshake)
- Traditional TCP + TLS 1.2:
  - 1 RTT for TCP 3-way handshake (`SYN`, `SYN-ACK`, `ACK`).
  - 2 RTTs for TLS 1.2 handshake.
  - Total = **3 RTTs** before the first byte of HTTP data!
- TCP + TLS 1.3:
  - 1 RTT TCP + 1 RTT TLS = **2 RTTs** (or 1 RTT on resumption).
- **QUIC + TLS 1.3**:
  - Combines transport connection establishment and cryptographic key exchange into a single unified handshake.
  - **First-time connection**: **1 RTT**.
  - **Repeat connection (0-RTT)**: The client uses cached session tickets and pre-shared keys to encrypt and send the HTTP GET request in the **very first UDP packet** ($0 \text{ RTT}$ delay!).

### 3. Connection ID (CID) & Seamless Connection Migration
- TCP identifies a connection by the 4-tuple `(Src IP, Src Port, Dst IP, Dst Port)`.
  - When a user walks out of their house and their smartphone transitions from home Wi-Fi to 5G cellular, the phone's IP address changes.
  - Every active TCP connection is severed, forcing applications (video calls, large downloads) to disconnect, re-resolve DNS, and re-establish from scratch.
- **QUIC identifies connections by a 64-bit Connection ID (CID)**:
  - The IP and port can change completely, but the server matches the packet by its CID.
  - The connection migrates seamlessly without dropping a single packet or renegotiating crypto keys!

### 4. Immunity to Middlebox Ossification
- Intermediate Internet middleboxes (firewalls, NAT routers, deep-packet inspection boxes) often inspect and tamper with TCP headers (e.g. dropping packets with unknown TCP options).
- Because TCP is baked into OS kernels, deploying TCP upgrades (like Multipath TCP or BBR) takes decades.
- In QUIC:
  - Built into **user space** (applications ship their own QUIC libraries).
  - Almost the **entire QUIC header and all payloads are encrypted** using authenticated encryption (AEAD).
  - Intermediate routers see only opaque UDP packets and cannot inspect or interfere with stream IDs, sequence numbers, or congestion parameters.
