# Visual Explanation: Head-of-Line Blocking and Handshake RTTs

---

## 1. Visual Comparison: HTTP/2 over TCP vs HTTP/3 over QUIC

```
HTTP/2 OVER TCP (Single Shared Byte Stream)
+-----------------------------------------------------------------------------+
| Stream 1 [Pkt 1] | Stream 2 [Pkt 1] | Stream 1 [Pkt 2] | Stream 3 [Pkt 1]   |
+-----------------------------------------------------------------------------+
                           |
                     PACKET DROPPED!
                           v
+-----------------------------------------------------------------------------+
| Stream 1 [Pkt 1] |   [ HOLE ]       | [ BLOCKED ]      | [ BLOCKED ]        |
+-----------------------------------------------------------------------------+
                          |
                          v
Receiver Kernel Buffer:
All subsequent packets for Stream 1 and Stream 3 are received, but CANNOT be
delivered to user-space because TCP must preserve the sequential byte stream!
>>> Entire website render stalls for 1 RTT until retransmission arrives! <<<


HTTP/3 OVER QUIC (Independent Logical Streams)
+-----------------------------------------------------------------------------+
| Packet 10: Stream 1 (Offset 0) | Packet 11: Stream 2 (Offset 0)             |
| Packet 12: Stream 1 (Offset 1) | Packet 13: Stream 3 (Offset 0)             |
+-----------------------------------------------------------------------------+
                                       |
                                 PACKET 11 DROPPED!
                                       v
Receiver QUIC Stack:
  - Stream 1: Delivers immediately to application! (Complete)
  - Stream 3: Delivers immediately to application! (Complete)
  - Stream 2: Only Stream 2 pauses waiting for retransmitted packet!
>>> 66% of the website components render with ZERO delay! <<<
```

---

## 2. Connection Handshake Comparison: TCP+TLS vs QUIC

```
TCP + TLS 1.3 (2 RTTs Minimum)                  QUIC (1 RTT Initial, 0 RTT Resumption)
Client                     Server              Client                     Server
  |                          |                   |                          |
  |--- 1. TCP SYN ---------->|                   |--- 1. QUIC Initial ----->|
  |<-- 2. TCP SYN-ACK -------|                   |    (TLS 1.3 ClientHello) |
  |--- 3. TCP ACK ---------->|                   |                          |
  |    + TLS ClientHello     |                   |<-- 2. QUIC Handshake ----|
  |<-- 4. TLS ServerHello ---|                   |    (TLS 1.3 ServerHello) |
  |                          |                   |                          |
  |--- 5. First HTTP GET --->|                   |--- 3. First HTTP GET --->|
  |<-- 6. HTTP 200 OK -------|                   |<-- 4. HTTP 200 OK -------|
  |                          |                   |                          |
  [ Total: 2 Full RTTs ]                         [ Total: 1 Single RTT ]
                                                 (Repeat visitors: 0 RTT!)
```
