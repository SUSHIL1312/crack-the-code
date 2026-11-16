# Theoretical Foundations: TCP Internals & Congestion Control

---

## 1. The 3-Way Handshake & Sequence Number Mechanics

Before any application bytes can be sent over TCP, client and server establish synchronized sequence numbers and negotiate protocol parameters.

```
Client (Port 54321)                                        Server (Port 80)
       |                                                           |
       | 1. SYN [Seq = ISN_C, Options: MSS=1460, WScale=7, SACK]   |
       +---------------------------------------------------------->| (Server enters SYN_RCVD)
       |                                                           |
       | 2. SYN-ACK [Seq = ISN_S, Ack = ISN_C + 1, Options]        |
       |<----------------------------------------------------------+
       | (Client enters ESTABLISHED)                               |
       |                                                           |
       | 3. ACK [Seq = ISN_C + 1, Ack = ISN_S + 1]                 |
       +---------------------------------------------------------->| (Server enters ESTABLISHED)
```

### Critical Handshake Concepts:
1. **Initial Sequence Number (ISN) Randomization**:
   - Sequence numbers must **never** start at 0.
   - If sequence numbers were predictable, an attacker could forge spoofed TCP packets and inject arbitrary data or hijack connections (Kevin Mitnick IP spoofing attack).
   - Modern kernels generate ISNs using a cryptographic hash:
     $$\text{ISN} = M + \text{HMAC}(\text{Src IP}, \text{Dst IP}, \text{Src Port}, \text{Dst Port}, \text{Secret Key})$$
2. **SYN and FIN Consume 1 Sequence Number**:
   - Even though a SYN or FIN packet carries 0 bytes of application payload, each consumes exactly 1 sequence number so that the other endpoint can explicitly acknowledge its receipt (`Ack = Seq + 1`).
3. **TCP Options Negotiated**:
   - **MSS (Maximum Segment Size)**: Typically 1460 bytes for IPv4 ($1500 - 20 - 20$).
   - **Window Scale (RFC 7323)**: Allows the 16-bit window size field to be shifted left by up to 14 bits, scaling receive buffers up to $1 \text{ GB}$ for high-BDP links.
   - **SACK Permitted (RFC 2018)**: Selective Acknowledgment allowing the receiver to acknowledge non-contiguous blocks of received packets.

---

## 2. Sliding Window Flow Control

Flow control is an **end-to-end mechanism** that prevents a fast sender from overrunning a slow receiver's memory buffer.

- The receiver advertises its available buffer space in the **Receive Window (`rwnd`)** field of every ACK packet.
- The sender's usable sending window is constrained by:
  $$\text{Effective Window} = \min(\text{cwnd}, \text{rwnd}) - (\text{LastByteSent} - \text{LastByteAcked})$$
- **Zero-Window Probing (Persist Timer)**:
  - If the receiver's application is slow reading data, its buffer fills up and it advertises `rwnd = 0` (Zero Window).
  - The sender stops transmitting application data.
  - *Deadlock Danger*: If the receiver later drains its buffer and sends a window update packet (`rwnd = 64KB`) that is lost on the network, both sides would wait forever!
  - *Solution*: The sender arms a **Persist Timer**. When it fires, the sender sends a 1-byte **Zero-Window Probe (ZWP)** packet. The receiver responds with an ACK advertising its current `rwnd`.

---

## 3. Connection Teardown & The TIME_WAIT State

TCP teardown is a 4-way handshake because TCP is full-duplex (each direction closes independently):

```
Endpoint A (Active Closer)                                Endpoint B (Passive Closer)
       |                                                           |
       | 1. FIN [Seq = u]                                          |
       +---------------------------------------------------------->| (B enters CLOSE_WAIT)
       | (A enters FIN_WAIT_1)                                     |
       |                                                           |
       | 2. ACK [Ack = u + 1]                                      |
       |<----------------------------------------------------------+
       | (A enters FIN_WAIT_2)                                     |
       |                                                           | 3. FIN [Seq = v]
       |<----------------------------------------------------------+ (B enters LAST_ACK)
       |                                                           |
       | 4. ACK [Ack = v + 1]                                      |
       +---------------------------------------------------------->| (B enters CLOSED)
       | (A enters TIME_WAIT for 2*MSL)                            |
       |                                                           |
```

### Why is `TIME_WAIT` Strictly Necessary?
The endpoint that initiates the active close enters `TIME_WAIT` and remains there for $2 \times \text{MSL}$ (typically $2 \times 30\text{s} = 60\text{s}$ on Linux).

1. **Reliable Connection Termination**:
   - What if Packet 4 (the final ACK sent by A) is lost on the wire?
   - Endpoint B will retransmit its FIN packet (Packet 3).
   - If Endpoint A did not have `TIME_WAIT` and closed immediately, Endpoint A would respond with a `RST` (connection reset) because the port is closed, making Endpoint B report an abnormal connection error rather than a graceful closure!
   - In `TIME_WAIT`, Endpoint A remembers the connection and can retransmit the final ACK.
2. **Preventing Old Duplicate Segments from Corrupting New Connections**:
   - Imagine Host A closes the connection and immediately reopens a new connection using the exact same 4-tuple `(A_IP, A_port, B_IP, B_port)`.
   - If an old delayed packet from the previous connection finally arrives across an errant router, its sequence numbers could match the new connection, silently injecting corrupt data into the new application stream!
   - Holding `TIME_WAIT` for $2 \times \text{MSL}$ ensures all delayed segments from the previous incarnation have expired and drained from the global Internet.

---

## 4. Congestion Control: Protecting the Shared Internet

While Flow Control (`rwnd`) protects the receiver, **Congestion Control (`cwnd`)** protects the shared intermediate router buffers from catastrophic congestion collapse.

### 1. Slow Start
- Starts with an initial window (typically $\text{IW} = 10 \text{ MSS} \approx 14.6 \text{ KB}$ per RFC 6928).
- For every ACK received, $cwnd$ increases by 1 MSS:
  $$cwnd \gets cwnd + \text{MSS}$$
- **Result**: $cwnd$ doubles every Round-Trip Time (RTT) $\implies$ **Exponential Growth** ($10 \to 20 \to 40 \to 80 \text{ MSS}$).
- Stops when $cwnd$ reaches the **Slow Start Threshold (`ssthresh`)** or packet loss occurs.

### 2. Congestion Avoidance (AIMD)
- **Additive Increase**: Once $cwnd \ge ssthresh$, growth switches to linear probing:
  $$cwnd \gets cwnd + \frac{\text{MSS} \times \text{MSS}}{cwnd} \quad (\approx +1 \text{ MSS per RTT})$$
- Probes network capacity conservatively.

### 3. Fast Retransmit & Fast Recovery
- If a single packet is lost while subsequent packets arrive, the receiver sends a **Duplicate ACK** for every out-of-order packet.
- Upon receiving **3 Duplicate ACKs**:
  - The sender knows a specific packet was lost without waiting for the slow Retransmission Timeout (RTO) timer.
  - Immediately retransmits the missing segment (**Fast Retransmit**).
  - Sets $ssthresh = cwnd / 2$.
  - Enters **Fast Recovery** (inflates window temporarily to keep pipeline flowing until loss is resolved).

---

## 5. Modern Algorithms: Cubic vs BBR

### TCP Cubic (Default in Linux since 2006)
- Replaces linear AIMD with a **cubic function of elapsed time $t$** since the last packet drop:
  $$W_{\text{cubic}}(t) = C \cdot (t - K)^3 + W_{\text{max}}$$
- **Shape**:
  1. Rapidly scales back up to near $W_{\text{max}}$ (concave phase).
  2. Stabilizes near $W_{\text{max}}$ where bandwidth was recently saturated.
  3. Gently probes for newly available capacity (convex phase).
- **Flaw**: Like Reno, Cubic is **loss-based**. It assumes packet loss equals congestion. In modern networks with large router buffers, Cubic fills up buffers until they overflow, causing massive **Bufferbloat** (latency spikes of 500ms+ on loaded links).

### Google BBR (Bottleneck Bandwidth and RTT - 2016)
- **Paradigm Shift**: Model-based, not loss-based!
- BBR operates at the optimal **Kleinrock Operating Point**:
  - Maximizes throughput (equals physical bottleneck bandwidth $B_{\text{btl}}$).
  - Minimizes delay (equals round-trip propagation time $\text{RTT}_{\text{prop}}$).
- **In-flight cap**: $\text{BDP} = B_{\text{btl}} \times \text{RTT}_{\text{prop}}$.
- BBR deliberately prevents queues from forming in routers, achieving **10x to 100x lower latency** and immunity to non-congestion packet loss (e.g. Wi-Fi bit errors).
