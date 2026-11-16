# Solutions: TCP Internals & Congestion Control Exercises

---

### Solution 1: Sequence Number Arithmetic & SACK

1. **Segment 1 received (Bytes 10,000 - 10,999)**:
   - Receiver sends: $\text{ACK} = 11,000$ (acknowledges all bytes up to 10,999; expects byte 11,000 next).

2. **Segment 3 received (Bytes 12,000 - 12,999; Segment 2 missing!)**:
   - Because TCP ACKs are **cumulative**, the receiver cannot acknowledge byte 12,000 without byte 11,000.
   - Receiver sends Duplicate ACK: $\text{ACK} = 11,000$.
   - **SACK Option**: Attaches SACK block $[12,000, 13,000)$ informing sender that bytes 12,000 to 12,999 are already buffered in RAM.

3. **Segment 4 received (Bytes 13,000 - 13,999)**:
   - Receiver sends Duplicate ACK: $\text{ACK} = 11,000$.
   - **SACK Option**: Expands SACK block to $[12,000, 14,000)$.

4. **Segment 2 retransmitted & received (Bytes 11,000 - 11,999)**:
   - Segment 2 fills the hole in the receiver's out-of-order queue.
   - The contiguous sequence is now complete from 10,000 all the way through 14,999!
   - Receiver returns cumulative: $\mathbf{\text{ACK} = 15,000}$ (expects byte 15,000 next). SACK blocks are cleared.

---

### Solution 2: Jacobson/Karels RTO Calculation

- $\text{SRTT}_{\text{prev}} = 100 \text{ ms}$, $\text{RTTVAR}_{\text{prev}} = 20 \text{ ms}$, sample $R = 180 \text{ ms}$.
- $\alpha = 0.125$, $\beta = 0.25$.

1. **New Smoothed RTT**:
   $$\text{SRTT}_{\text{new}} = (1 - 0.125) \times 100 + 0.125 \times 180 = 87.5 + 22.5 = \mathbf{110.0 \text{ ms}}$$

2. **New RTT Variation**:
   - Difference: $|100 - 180| = 80 \text{ ms}$.
   $$\text{RTTVAR}_{\text{new}} = (1 - 0.25) \times 20 + 0.25 \times 80 = 15 + 20 = \mathbf{35.0 \text{ ms}}$$

3. **Updated RTO**:
   $$\text{RTO} = \text{SRTT}_{\text{new}} + 4 \times \text{RTTVAR}_{\text{new}} = 110 + (4 \times 35) = 110 + 140 = \mathbf{250.0 \text{ ms}}$$

4. **Exponential Backoff**:
   - If the retransmission times out, RTO doubles: $250 \times 2 = \mathbf{500.0 \text{ ms}}$.

---

### Solution 3: Bufferbloat Calculation

1. **Bandwidth-Delay Product (BDP)**:
   $$\text{BDP} = 100 \text{ Mbps} \times 0.020 \text{ s} = 2 \text{ Megabits} = \frac{2 \times 10^6}{8 \times 10^6} = \mathbf{0.25 \text{ MB}} \ (250 \text{ KB})$$

2. **Queuing Delay in 10 MB Buffer**:
   - Time to drain 10 MB at 100 Mbps ($12.5 \text{ MB/s}$):
     $$D_{\text{queue}} = \frac{10 \text{ MB}}{12.5 \text{ MB/s}} = \mathbf{0.800 \text{ seconds}} = 800 \text{ ms}$$
   - Observed RTT:
     $$\text{RTT}_{\text{bloated}} = \text{Unloaded RTT} + D_{\text{queue}} = 20 \text{ ms} + 800 \text{ ms} = \mathbf{820 \text{ ms}}$$
   - Latency jumps from $20 \text{ ms}$ to nearly **1 whole second** ($41\times$ increase!), ruining voice/video calls while a download is running.

3. **How BBR Eliminates Bufferbloat**:
   - Instead of increasing $cwnd$ until packets are dropped (filling the 10 MB buffer), BBR actively measures the true physical bottleneck bandwidth ($B_{\text{btl}} = 100 \text{ Mbps}$) and minimum propagation RTT ($20 \text{ ms}$).
   - BBR caps inflight data to $\text{BDP} = 250 \text{ KB}$ and paces transmission rate to exactly match $100 \text{ Mbps}$.
   - Because no excess packets enter the router queue, the 10 MB buffer remains empty $\implies$ ping stays at $20 \text{ ms}$ while utilizing $100\%$ of the 100 Mbps link!
