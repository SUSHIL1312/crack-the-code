# Theoretical Foundations: The Transport Layer

---

## 1. Process-to-Process Multiplexing

Without the Transport Layer, a computer could only run one network application at a time. The Transport Layer solves this using **Port Numbers** (16-bit unsigned integers from $0$ to $65,535$):

### Port Number Ranges (IANA Standard):
1. **Well-Known Ports ($0 - 1023$)**: Reserved for standard system services. Requires root/privileged execution on POSIX systems (`CAP_NET_BIND_SERVICE`).
   - `22`: SSH
   - `53`: DNS
   - `80`: HTTP
   - `443`: HTTPS
2. **Registered Ports ($1024 - 49151$)**: Assigned for user applications and databases.
   - `3306`: MySQL
   - `5432`: PostgreSQL
   - `6379`: Redis
   - `8080`: HTTP alternative proxy
3. **Dynamic / Ephemeral Ports ($49152 - 65535$)**: Assigned automatically by the OS kernel to outbound client connections. (On Linux, configurable via `/proc/sys/net/ipv4/ip_local_port_range`, typically `32768 - 60999`).

---

## 2. Socket Demultiplexing: TCP vs UDP

### Connectionless Demultiplexing (UDP)
A UDP socket is identified by a **2-tuple**:
$$(\text{Destination IP}, \text{Destination Port})$$
- When an incoming UDP datagram arrives at Host B, the OS kernel checks only the destination port.
- If two different clients (Host A1 with IP 1.1.1.1, port 5000 and Host A2 with IP 2.2.2.2, port 6000) both send UDP datagrams to Host B on port 53 (DNS server), **both datagrams are delivered to the exact same socket file descriptor**.
- The server reads the sender's identity using `recvfrom()`.

### Connection-Oriented Demultiplexing (TCP)
A TCP socket is identified by a **4-tuple**:
$$(\text{Source IP}, \text{Source Port}, \text{Destination IP}, \text{Destination Port})$$
- A web server listening on port 80 has a single **listening socket**.
- When Client A connects from `(192.168.1.50, 52100)` and Client B connects from `(192.168.1.60, 52100)`, the kernel's `accept()` system call creates **two completely distinct socket file descriptors**.
- All subsequent incoming packets from Client A are steered to Socket 1, while packets from Client B are steered to Socket 2, even though both share destination port 80 and both clients happened to use source port 52100!

---

## 3. UDP: User Datagram Protocol (RFC 768)

UDP is a minimal, "no-frills" transport protocol. It provides only two services beyond raw IP:
1. **Port Multiplexing** (delivering bytes to the right process).
2. **Error Detection** (16-bit checksum).

### Header Layout (8 Bytes):
```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Source Port          |       Destination Port        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|            Length             |           Checksum            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
- **Length**: Length in bytes of UDP header + UDP payload (minimum 8 bytes).
- **Checksum**: Optional in IPv4 (set to `0x0000` if unused), mandatory in IPv6.

### Why Choose UDP?
- **Zero Connection Setup Delay**: No 3-way handshake (0 RTT).
- **No Head-of-Line Blocking**: One lost packet does not delay subsequent packets.
- **No Congestion Control**: Transmits at the exact rate requested by application (critical for live video streaming, WebRTC, real-time gaming, DNS).
- **Message Boundary Preservation**: One `sendto()` generates exactly one `recvfrom()`.

---

## 4. TCP vs UDP Architectural Comparison

| Feature | UDP (RFC 768) | TCP (RFC 793) |
| :--- | :--- | :--- |
| **Connection State** | Connectionless (no handshake, no teardown) | Connection-Oriented (3-way handshake, 4-way FIN) |
| **Data Delivery** | Unreliable (best-effort, drops packets) | Reliable (ACKs, retransmissions, timeouts) |
| **Ordering** | Unordered (packets can arrive out-of-order) | Strictly Ordered (sequence numbers, reassembly buffer) |
| **Data Stream** | Discrete Datagrams (preserves message boundaries) | Continuous Byte Stream (no message boundaries) |
| **Flow Control** | None (can overrun receiver buffer) | Yes (sliding window advertised by receiver) |
| **Congestion Control** | None (sends at arbitrary rate) | Yes (AIMD, Slow Start, Reno, Cubic, BBR) |
| **Header Overhead** | 8 bytes | 20 to 60 bytes |
| **Typical Use Cases** | DNS, DHCP, WebRTC, VoIP, QUIC/HTTP/3 | Web (HTTP/1.1, HTTP/2), SSH, TLS, Database SQL |
