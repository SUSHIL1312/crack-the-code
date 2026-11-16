# The Transport Layer: Top FAANG Interview Questions

### Q1: Can two completely different TCP connections share the exact same local port number on the same computer?
**Answer**:
- **Yes, absolutely!**
- **Explanation**: A TCP connection is uniquely identified by the **complete 4-tuple**:
  $$(\text{Source IP}, \text{Source Port}, \text{Destination IP}, \text{Destination Port})$$
  - As long as at least **one** of the four values differs, the kernel considers them completely distinct sockets.
- **Examples**:
  1. **Server Side**: A web server listening on port 80 connects to 50,000 different clients. All 50,000 connections share local port 80 (`Dest Port = 80`), but each has a unique client `(Source IP, Source Port)`.
  2. **Client Side**: A client process at `192.168.1.50` using ephemeral port `52000` can connect to Web Server 1 (`93.184.216.34:80`) and Web Server 2 (`142.250.190.46:80`) using the **exact same local port 52000**. Because the destination IPs differ, the 4-tuples are unique and there is no collision.

---

### Q2: Why does TCP use a byte-stream model while UDP uses a datagram model? What is the impact on application message framing?
**Answer**:
- **UDP (Datagram Model)**:
  - Preserves application message boundaries.
  - If the sender calls `sendto(fd, buf, 100)`, the receiver's single `recvfrom(fd, recv_buf, 200)` returns exactly 100 bytes (one full message).
  - The application does not need to delimit message boundaries in the payload.
- **TCP (Byte-Stream Model)**:
  - Has **zero concept of message boundaries**. To TCP, data is an arbitrary, unstructured stream of bytes.
  - If the sender calls `write(fd, "HELLO", 5)` followed by `write(fd, "WORLD", 5)`:
    - The receiver calling `read(fd, buf, 1024)` might receive all 10 bytes at once (`"HELLOWORLD"`).
    - Or receive 3 bytes (`"HEL"`), then 4 bytes (`"LOWO"`), then 3 bytes (`"RLD"`).
- **Impact on Application Protocols**:
  - Every application protocol running over TCP **must implement its own message framing**:
    1. **Delimiters**: E.g., HTTP headers use `\r\n\r\n`, Redis uses `\r\n`.
    2. **Length-Prefixed Framing**: E.g., gRPC / Protobuf, Kafka, and custom binary protocols prepend a 4-byte integer length indicating the size of the subsequent payload.

---

### Q3: When should an engineer choose UDP over TCP?
**Answer**:
1. **Low Latency / Real-Time Media**: VoIP, video conferencing (Zoom, WebRTC), and multiplayer gaming. Dropping an audio packet from 200 ms ago is far preferable to stalling the entire audio stream waiting for TCP retransmission.
2. **Simple Request-Reply Transactions**: DNS (`Port 53`), DHCP (`Port 67/68`), NTP (`Port 123`). Incurring a 3-way TCP handshake to fetch a 60-byte DNS record doubles lookup latency. If a UDP packet is lost, the client simply retransmits after a timeout.
3. **Multicast and Broadcast**: TCP is fundamentally point-to-point (unicast only). Applications broadcasting to LAN segments or multicasting video feeds to thousands of subscribers (IPTV) must use UDP.
4. **Custom Application-Level Reliability**: Protocols like **QUIC (HTTP/3)** use UDP as a lightweight transport substrate and implement their own user-space encryption, congestion control, and stream multiplexing without kernel TCP limitations.
