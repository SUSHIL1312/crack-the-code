# UDP & QUIC (HTTP/3): Top FAANG Interview Questions

### Q1: Why did Google build QUIC on top of UDP instead of proposing a brand-new Layer 4 protocol (like SCTP)?
**Answer**:
1. **Middlebox Ossification**:
   - The Internet is filled with millions of enterprise firewalls, NAT routers, and carrier gateways that only recognize IP Protocol 6 (TCP) and Protocol 17 (UDP).
   - Past attempts to introduce new transport protocols (such as SCTP - Stream Control Transmission Protocol, or DCCP) completely failed because > 30% of commercial middleboxes silently dropped any packet with an unknown IP protocol number!
   - By encapsulating inside UDP (Protocol 17), QUIC packets traverse all existing internet firewalls and NAT gateways without needing a single hardware upgrade worldwide.
2. **Kernel Deployment Velocity**:
   - Changing TCP requires upgrading the OS kernel on billions of devices (Windows, Android, iOS, macOS, Linux). Kernel upgrades take 5 to 10 years to reach global ubiquity.
   - By running on top of UDP in **user space**, browser vendors (Google Chrome, Mozilla Firefox) and cloud providers (Cloudflare, Meta, Google) can ship QUIC protocol improvements, new congestion control algorithms, and bug fixes in a weekly browser update!

---

### Q2: What is Transport-Layer Head-of-Line (HoL) Blocking in HTTP/2, and how does QUIC eliminate it?
**Answer**:
- **In HTTP/2 over TCP**:
  - All logical HTTP requests are multiplexed into a single TCP connection.
  - The TCP kernel subsystem treats all bytes as one single serialized queue.
  - If a packet containing 1,400 bytes of CSS is dropped by Wi-Fi noise, the TCP stack will **not** deliver subsequent packets containing JavaScript or HTML to the browser until the CSS packet is retransmitted.
  - A single dropped packet stalls **all concurrent streams**.
- **In QUIC**:
  - QUIC understands stream boundaries at the transport layer.
  - Every stream has its own independent stream offset.
  - If a packet carrying Stream 1 data is lost, only Stream 1 pauses. Streams 2, 3, and 4 continue delivering bytes to the application without interruption.

---

### Q3: What is a 0-RTT Replay Attack in QUIC / TLS 1.3, and how can backend engineers protect against it?
**Answer**:
- **Vulnerability**:
  - In 0-RTT resumption, the client transmits application data in its very first packet using keys derived from a previous session ticket.
  - Because the server has not yet exchanged fresh cryptographic nonces, an active network adversary can sniff the 0-RTT packet and replay it multiple times to the server.
- **Protection Rules**:
  1. **Strict Idempotency**:
     - Never permit non-idempotent HTTP methods (`POST`, `PUT`, `DELETE`, `PATCH`) in 0-RTT early data.
     - Only allow safe, idempotent `GET` or `HEAD` queries.
  2. **Single-Use Strike Register**:
     - The server maintains an in-memory cache (e.g. Redis bloom filter or distributed hash table) of recently observed client nonces / tickets. If a duplicate ticket arrives within the validity window, it is rejected.
  3. **Client-Hello Age Verification**:
     - The server rejects any 0-RTT data whose timestamp deviates by more than a few seconds from the server's local clock.
