# Theoretical Foundations: Networking Fundamentals

---

## 1. Switching Paradigms: Circuit vs Packet Switching

### Circuit Switching (Legacy Telephony, SDH/SONET)
- **Concept**: A dedicated physical or time-division multiplexed (TDM) path is reserved between endpoints for the entire duration of the session.
- **Advantages**: Guaranteed constant bandwidth, deterministic zero queuing delay, no packet drops or reordering.
- **Disadvantages**: Inefficient utilization during quiet periods (wasted bandwidth), high connection setup latency, cannot scale to millions of concurrent bursty web connections.

### Packet Switching (The Modern Internet)
- **Concept**: Messages are divided into discrete chunks called **packets**. Each packet contains header metadata (source, destination, sequence number). Packets travel independently through intermediate routers using **statistical multiplexing**.
- **Advantages**: High channel efficiency (idle capacity is immediately used by other flows), dynamic rerouting around failed links, massive scalability.
- **Disadvantages**: Variable latency (jitter), packet loss under buffer congestion, out-of-order delivery requiring reassembly buffers at the destination.

---

## 2. The Four Fundamental Sources of Network Delay

The total end-to-end delay for a single packet traveling through a series of $N$ hops is:

$$D_{\text{total}} = \sum_{i=1}^{N} \left( D_{\text{proc}, i} + D_{\text{queue}, i} + D_{\text{trans}, i} + D_{\text{prop}, i} \right)$$

### 1. Nodal Processing Delay ($D_{\text{proc}}$)
- **Cause**: Time taken by the router's network processor to inspect the packet header, verify the IP checksum, look up the destination IP in the routing table (FIB), and direct the packet to the output interface queue.
- **Magnitude**: Typically $< 1 \text{ to } 10 \text{ microseconds}$ on hardware ASICs.

### 2. Queuing Delay ($D_{\text{queue}}$)
- **Cause**: Time the packet spends sitting in the router's output interface buffer waiting to be transmitted over the physical wire.
- **Traffic Intensity**: Let $a$ be packet arrival rate, $L$ be packet length in bits, and $R$ be link transmission rate in bits/sec.
  $$\text{Traffic Intensity } I = \frac{L \cdot a}{R}$$
  - If $I \approx 0$: Queuing delay is near zero.
  - As $I \to 1$: Queuing delay increases exponentially towards infinity.
  - If $I > 1$: Arrival rate exceeds transmission capacity; packet buffers fill up, resulting in **packet drops** (tail drop or RED).

### 3. Transmission Delay ($D_{\text{trans}}$)
- **Cause**: Time required to push all bits of the packet onto the physical medium.
- **Formula**:
  $$D_{\text{trans}} = \frac{L}{R}$$
  where $L$ is packet size in bits, and $R$ is link transmission rate (bandwidth) in bits per second (bps).
- **Example**: Pushing a 1500-byte Ethernet packet ($1500 \times 8 = 12,000 \text{ bits}$) onto a 1 Gbps link:
  $$D_{\text{trans}} = \frac{12,000}{10^9} = 12 \text{ microseconds}$$

### 4. Propagation Delay ($D_{\text{prop}}$)
- **Cause**: Time required for a single physical electromagnetic bit to travel across the physical medium from sender to receiver.
- **Formula**:
  $$D_{\text{prop}} = \frac{d}{s}$$
  where $d$ is physical cable distance, and $s$ is the propagation speed in the medium.
  - Speed of light in vacuum: $c \approx 3 \times 10^8 \text{ m/s}$.
  - Speed of light in fiber optic glass ($n \approx 1.5$):
    $$s \approx \frac{c}{1.5} \approx 2 \times 10^8 \text{ m/s} = 200 \text{ km/millisecond}$$
- **Key Insight**: Transmission delay is determined by **packet size and link rate**; propagation delay is determined strictly by **physical distance and the speed of light**.

---

## 3. Bandwidth vs Throughput vs Goodput

| Metric | Definition | Analogy |
| :--- | :--- | :--- |
| **Bandwidth** | Maximum theoretical theoretical data rate a link can sustain (e.g. 10 Gbps Ethernet). | Width of the water pipe. |
| **Throughput** | Actual measured rate at which data successfully passes through the link over time. | Actual liters of water flowing per second (accounting for friction and valves). |
| **Goodput** | Actual rate of **useful application payload** delivered (excluding TCP/IP/Ethernet header overhead and retransmitted packets). | Net clean water delivered to the glass (excluding splash and pipe leakage). |

---

## 4. Jitter and Packet Loss
- **Jitter**: The statistical variance in packet arrival latency ($\Delta \text{RTT}$). Critical for real-time media (VoIP, video calls, gaming). High jitter causes audio stutter and requires jitter playout buffers.
- **Packet Loss**: Fraction of packets discarded due to full router queues, bit errors, or link degradation. Packet loss forces TCP retransmission, triggering congestion control window collapse.
