# Internal Working: QUIC Packet Formats and Stream Frames

---

## 1. QUIC Packet Architecture

QUIC packets are encapsulated directly inside UDP datagrams. A single UDP datagram can contain multiple bundled QUIC packets.

QUIC defines two header formats:
1. **Long Header**: Used during connection establishment (Initial, 0-RTT, Handshake, Retry).
2. **Short Header (1-RTT)**: Used during steady-state data transfer after keys are established.

### Short Header (1-RTT) Layout:
```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|0|1|S|R|R|K|P P|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                Destination Connection ID (0..160 bits)        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Packet Number (8..32 bits)                |  <-- Encrypted!
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Protected Payload (Frames)                  |  <-- AEAD Encrypted!
|                              ...                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- **Header Protection**: Notice that even the **Packet Number is encrypted**! Middleboxes cannot track packet numbers to infer packet loss rates or latency.
- **Monotonically Increasing Packet Numbers**: Unlike TCP (where retransmitted packets reuse the exact same sequence number), QUIC **never reuses a packet number**. Every retransmitted packet receives a strictly new, higher packet number. This completely eliminates TCP's ambiguous RTT retransmission problem (Karn's algorithm).

---

## 2. QUIC Stream Frames (`STREAM_FRAME`)

Inside the encrypted payload of a QUIC packet, data is partitioned into discrete **Frames**:

```
+---------------------------------------------------------------+
|  Type (0x08 - 0x0F)  |  Stream ID (variable)  |  Offset (var) |
+---------------------------------------------------------------+
|  Length (variable)   |  Stream Data (Bytes)                   |
+---------------------------------------------------------------+
```

- **Stream ID**: Identifies which logical flow this data belongs to (e.g. Stream 4 = `avatar.png`, Stream 8 = `app.js`).
  - Bit 0: Initiator (`0` = Client-initiated, `1` = Server-initiated).
  - Bit 1: Directionality (`0` = Bidirectional, `1` = Unidirectional).
- **Offset**: Byte offset within that specific stream (analogous to TCP sequence number, but strictly isolated per stream).
- **Fin Bit**: Marks the end of that specific stream.

When a packet arrives containing Stream Frames for Stream 4 and Stream 8, if Stream 4 had a previous packet lost, the QUIC library buffers Stream 4's data while **immediately delivering Stream 8's data to the application layer**.

---

## 3. Cryptographic Key Schedule & 0-RTT Replay Protection

When a client reconnects to a server using 0-RTT:
1. The client derives an early secret from a cached **Resumption Pre-Shared Key (PSK)** obtained from a previous TLS 1.3 `NewSessionTicket`.
2. Encrypts the 0-RTT application data with the early key and sends it in the first packet (`Initial + 0-RTT`).
3. **Replay Attack Risk**: An attacker eavesdropping on the network could capture the 0-RTT packet and replay it 1,000 times to repeat an action (e.g. replaying a `POST /transfer?amount=100` request).
4. **Defense Mechanisms**:
   - RFC 8470: 0-RTT data is restricted to **idempotent requests** (safe `GET` requests only; never non-idempotent `POST` or `DELETE`).
   - Server-side strike registers: Server caches incoming client nonces or rejects 0-RTT packets outside a small sliding time window ($\pm 10 \text{ seconds}$).
