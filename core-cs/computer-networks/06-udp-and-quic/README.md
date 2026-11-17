# Module 06: UDP, Head-of-Line Blocking, and QUIC (HTTP/3)

<!-- Chapter Navigation Header -->
| ⬅️ [Module 05: TCP Internals](../05-tcp-internals/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Module 07: Sockets Programming](../07-sockets-programming/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## Overview
For over 30 years, TCP powered nearly all web traffic. However, modern interactive mobile applications exposed fundamental architectural limitations in TCP: multi-RTT connection handshake latency, connection breakage during Wi-Fi to cellular handoffs, middlebox protocol ossification, and **Transport-Layer Head-of-Line (HoL) Blocking** in multiplexed protocols like HTTP/2.

To solve this, Google developed and IETF standardized **QUIC (RFC 9000)**, a modern transport protocol built directly on top of UDP that forms the foundation of **HTTP/3**.

In this module, you will master the mechanics of UDP, dissect the Head-of-Line blocking problem, and understand how QUIC achieves 0-RTT handshakes, independent stream multiplexing, and seamless connection migration.

---

## Learning Objectives
1. **UDP Mechanics**: Revisit UDP's zero-overhead datagram model and why it is the ideal substrate for user-space transport protocols.
2. **The Head-of-Line Blocking Problem**: Understand why multiplexing multiple logical streams over a single TCP connection (HTTP/2) causes all streams to stall when a single packet is dropped.
3. **QUIC Architecture (RFC 9000)**:
   - User-Space Transport over UDP.
   - Independent stream flow control (loss on Stream A never stalls Stream B).
   - Integrated TLS 1.3 Cryptographic Handshake (0-RTT connection resumption).
   - Connection ID (CID) and seamless mobile Connection Migration.
4. **Ossification Defense**: Understand how QUIC encrypts transport headers to prevent intermediate routers/middleboxes from tampering with protocol evolution.
5. **Hands-On Simulation**: Implement an empirical C++ benchmark comparing TCP vs QUIC stream completion times under packet loss.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of Head-of-Line blocking, QUIC protocol specification, and HTTP/3.
- `internal-working.md`: QUIC packet layouts, Connection IDs, 0-RTT keys, and stream frame demultiplexing.
- `visual-explanation.md`: Visual comparisons of HTTP/2 TCP HoL stalls vs HTTP/3 QUIC stream independence and connection migration.
- `examples/`:
  - `quic_vs_tcp_hol_simulator.cpp`: C++ simulator demonstrating how packet loss stalls all streams in TCP while QUIC delivers parallel streams in real time.
- `exercises/`: Handshake RTT analysis and packet loss modeling exercises.
- `solutions/`: Complete solutions and calculations.
- `interview-questions.md`: FAANG interview questions (e.g. "Why did Google build QUIC on UDP instead of creating a new IP protocol?", "Connection migration mechanics").
- `common-mistakes.md`: Believing QUIC is unreliable because it uses UDP, confusing HTTP/2 stream multiplexing with HTTP/3.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 05: TCP Internals](../05-tcp-internals/README.md) | [Computer Networks Hub](../README.md) | **[Module 07: Sockets Programming](../07-sockets-programming/README.md)** |
<!-- /Chapter Navigation Footer -->

