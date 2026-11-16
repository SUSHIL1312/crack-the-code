# Hands-On Exercises: TCP Internals & Congestion Control

---

### Exercise 1: TCP Sequence Number Arithmetic & SACK Block Decoding
**Goal**: Trace TCP byte offsets and Selective Acknowledgment (SACK) blocks under packet loss.

**Scenario**:
Sender transmits 5 consecutive TCP segments to Receiver, starting at Initial Sequence Number $\text{ISN} = 10,000$. Each segment contains $1,000$ bytes of application data:
- Segment 1: Bytes $10,000 - 10,999$
- Segment 2: Bytes $11,000 - 11,999$
- Segment 3: Bytes $12,000 - 12,999$
- Segment 4: Bytes $13,000 - 13,999$
- Segment 5: Bytes $14,000 - 14,999$
**Event**: Segment 2 is dropped by an intermediate router. Segments 1, 3, 4, and 5 arrive successfully at Receiver.
1. What ACK number does the receiver send in response to Segment 1?
2. What ACK number does the receiver send when Segment 3 arrives? What SACK block is attached?
3. What ACK number does the receiver send when Segment 4 arrives? What SACK block is attached?
4. When the sender retransmits Segment 2 and it arrives, what cumulative ACK number does Receiver return?

---

### Exercise 2: TCP Retransmission Timeout (RTO) Calculation
**Goal**: Implement the Jacobson/Karels algorithm by hand.

**Data**:
- Initial $\text{SRTT} = 100 \text{ ms}$, $\text{RTTVAR} = 20 \text{ ms}$.
- New RTT measurement for next packet: $R = 180 \text{ ms}$ (latency spike).
- Standard parameters: $\alpha = 0.125$, $\beta = 0.25$, clock granularity $G = 1 \text{ ms}$.
1. Compute the new Smoothed RTT ($\text{SRTT}_{\text{new}}$).
2. Compute the new RTT Variation ($\text{RTTVAR}_{\text{new}}$).
3. Compute the updated $\text{RTO}$.
4. If this packet is retransmitted and lost again, what will the backed-off RTO become?

---

### Exercise 3: Bufferbloat Calculation and BBR Analysis
**Goal**: Calculate latency degradation caused by oversized drop-tail router buffers.

**Scenario**:
A home fiber connection has:
- Bandwidth $R = 100 \text{ Mbps}$.
- Unloaded Round-Trip Propagation Delay $\text{RTT} = 20 \text{ ms}$.
- Home router has a 10 MB buffer.
1. Calculate the Bandwidth-Delay Product (BDP) in megabytes.
2. If TCP Cubic fills the router's 10 MB buffer, what does the observed ping latency (RTT) spike to?
3. How does Google BBR prevent this bufferbloat while still saturating the 100 Mbps link?
