# Module 04: The Transport Layer

<!-- Chapter Navigation Header -->
| ⬅️ [Module 03: Ethernet, ARP & Switching](../03-ethernet-arp-and-switching/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Module 05: TCP Internals](../05-tcp-internals/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## Overview
While the Network Layer (IP) provides host-to-host packet delivery across the global Internet, a host runs hundreds of concurrent processes—web browsers, background daemons, database connections, and streaming media. The **Transport Layer (Layer 4)** provides **process-to-process communication**, turning raw host-level packet delivery into dedicated, multiplexed application communication channels.

In this module, you will master port numbers, transport layer multiplexing/demultiplexing, the fundamental dichotomy between UDP and TCP, and implement the RFC 1071 Internet Checksum algorithm including the IPv4 Pseudo-Header in C++.

---

## Learning Objectives
1. **Process Multiplexing & Demultiplexing**: Understand how 16-bit port numbers ($0 - 65,535$) allow a single network card to serve thousands of concurrent application connections.
2. **Socket Demultiplexing**:
   - Connectionless Demultiplexing (UDP): 2-tuple `(Dest IP, Dest Port)`.
   - Connection-Oriented Demultiplexing (TCP): 4-tuple `(Src IP, Src Port, Dst IP, Dst Port)`.
3. **UDP Architecture**: Analyze the lightweight 8-byte UDP header, message boundary preservation, and when to use UDP.
4. **TCP Overview**: Contrast UDP datagrams with TCP's full-duplex, reliable, connection-oriented byte stream.
5. **The Internet Checksum (RFC 1071)**: Master ones' complement 16-bit addition, end-around carry arithmetic, and the IPv4 Pseudo-Header.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of transport multiplexing, port allocations, TCP vs UDP comparison, and checksum theory.
- `internal-working.md`: Socket hash tables in the Linux kernel (`inet_hashinfo`), the 12-byte IPv4 pseudo-header, and checksum offloading (CSO).
- `visual-explanation.md`: Visual diagrams of transport demultiplexing and ones' complement arithmetic.
- `examples/`:
  - `transport_checksum_demo.cpp`: Complete C++ implementation of the RFC 1071 Internet Checksum algorithm with pseudo-header verification and bit-flip corruption detection.
- `exercises/`: Socket demultiplexing scenarios and checksum calculation exercises.
- `solutions/`: Step-by-step solutions.
- `interview-questions.md`: FAANG interview questions (e.g., "Can two different TCP connections share the exact same port number?").
- `common-mistakes.md`: Ephemeral port exhaustion, forgetting pseudo-header in checksums, expecting UDP to preserve message ordering.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 03: Ethernet, ARP & Switching](../03-ethernet-arp-and-switching/README.md) | [Computer Networks Hub](../README.md) | **[Module 05: TCP Internals](../05-tcp-internals/README.md)** |
<!-- /Chapter Navigation Footer -->

