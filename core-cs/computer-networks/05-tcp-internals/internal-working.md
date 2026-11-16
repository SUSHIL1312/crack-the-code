# Internal Working: TCP Kernel Data Structures, RTO Math, & SYN Cookies

---

## 1. TCP Control Block in the Linux Kernel (`struct tcp_sock`)

In the Linux kernel, every TCP connection is represented by `struct tcp_sock` (extending `struct inet_connection_sock` and `struct sock`):

```c
struct tcp_sock {
    u32 snd_una;     // First unacknowledged sequence number
    u32 snd_nxt;     // Next sequence number to be sent
    u32 snd_wnd;     // Sender window advertised by peer
    u32 rcv_nxt;     // Next expected sequence number from peer
    u32 rcv_wnd;     // Current receive window advertised to peer

    u32 snd_cwnd;    // Congestion window (in MSS units or bytes)
    u32 snd_ssthresh;// Slow start threshold

    u32 srtt_us;     // Smoothed RTT (microsecond scale, Jacobson/Karels)
    u32 rttvar_us;   // RTT variance
    u32 rto;         // Retransmission Timeout (in jiffies)

    struct rb_root out_of_order_queue; // Red-Black tree holding out-of-order segments
    struct tcp_sack_block recv_sack_cache[4]; // SACK blocks for non-contiguous ACKs
    ...
};
```

---

## 2. Dynamic RTT Estimation & RTO Math (RFC 6298)

How does TCP know how long to wait before deciding a packet was lost and must be retransmitted?
A fixed timeout (e.g. 1 second) would either be catastrophically slow on datacenter LANs (where RTT is $10 \ \mu\text{s}$) or prematurely trigger false retransmissions on satellite/cellular links (where RTT is $500 \text{ ms}$).

TCP dynamically computes the **Retransmission Timeout (RTO)** using the **Jacobson/Karels Algorithm**:

1. Measure sample RTT ($R$) for a segment (using Karn's algorithm: never measure RTT for retransmitted packets!).
2. Update **Smoothed RTT ($SRTT$)**:
   $$SRTT \gets (1 - \alpha) \cdot SRTT + \alpha \cdot R \quad (\text{Standard } \alpha = \frac{1}{8} = 0.125)$$
3. Update **RTT Variation ($RTTVAR$)**:
   $$RTTVAR \gets (1 - \beta) \cdot RTTVAR + \beta \cdot |SRTT - R| \quad (\text{Standard } \beta = \frac{1}{4} = 0.25)$$
4. Compute **RTO**:
   $$\text{RTO} = SRTT + \max(G, 4 \cdot RTTVAR)$$
   where $G$ is the clock granularity.
5. **Exponential Backoff**: If an RTO timer expires and a packet is retransmitted, RTO doubles ($\text{RTO} \gets 2 \cdot \text{RTO}$) up to a maximum of 64 or 120 seconds.

---

## 3. Cryptographic SYN Cookies (RFC 4987)

Under a **SYN Flood Attack**, an attacker sends millions of spoofed TCP SYN packets with randomized fake source IPs.
- **Normal Server Behavior**: The server allocates a `struct request_sock` in kernel memory for each SYN, places it in the **SYN backlog queue**, and waits for the final ACK. The queue quickly overflows, causing the server to reject legitimate users.
- **SYN Cookies (Stateless Defense)**:
  - When the SYN backlog overflows, the kernel switches to SYN Cookies mode: **zero memory is allocated** for incoming SYNs!
  - The server encodes all critical connection state into the **32-bit Initial Sequence Number (ISN)**:

```
+---------------------------------------------------------------+
|  t (5 bits)  |  MSS Index (3 bits)  |  Cryptographic Hash (24 bits)  |
+---------------------------------------------------------------+
```

1. **`t` (5 bits)**: A slowly incrementing timestamp (advances every 64 seconds) to prevent replay attacks.
2. **MSS Index (3 bits)**: Encodes one of 8 standard MSS values (e.g. 536, 1200, 1460, 8960).
3. **Hash (24 bits)**:
   $$\text{Hash} = \text{SHA256}(\text{Src IP}, \text{Dst IP}, \text{Src Port}, \text{Dst Port}, t, \text{Secret Server Key}) \pmod{2^{24}}$$
4. **Verification**:
   - The server sends this computed ISN back in the `SYN-ACK`.
   - When the client responds with `ACK (ack_num = ISN + 1)`, the server subtracts 1 from the incoming acknowledgment number, recomputes the SHA-256 hash using the 4-tuple and timestamp $t$, and verifies the hash.
   - If valid, the connection is instantiated directly into the established socket table without ever having stored state in a SYN queue!
