# Networking Fundamentals: Top FAANG Interview Questions

### Q1: Why is the minimum ping latency between the US East Coast and India approximately 150-180 ms, regardless of ISP or technology?
**Answer**:
1. **Physical Distance**: The shortest surface distance along the Earth's curvature between New York and Mumbai is approximately $12,500 \text{ km}$.
2. **Speed of Light in Fiber**: Data travels through optical fiber where the refractive index of glass $n \approx 1.5$, meaning light propagates at $\approx 200,000 \text{ km/s}$ (about 33% slower than in a vacuum).
3. **One-Way Minimum Transit Time**:
   $$D_{\text{prop}} = \frac{12,500 \text{ km}}{200,000 \text{ km/s}} = 0.0625 \text{ s} = 62.5 \text{ ms}$$
4. **Round-Trip Time (RTT)**:
   $$\text{RTT}_{\text{ideal}} = 2 \times 62.5 \text{ ms} = 125 \text{ ms}$$
5. **Real-World Cable Geometry**: Submarine cables do not follow ideal straight lines. They route through the Atlantic, Mediterranean, Red Sea, and Arabian Sea (or across the Pacific), adding roughly 20-30% extra distance ($\approx 15,000 - 16,000 \text{ km}$).
6. **Active Routing Overhead**: Intermediate optical repeaters, BGP border routers, and queuing delays add an additional 15-25 ms.
- **Conclusion**: The speed of light in optical glass imposes a physical floor of $\approx 150 - 180 \text{ ms}$ that cannot be overcome by software engineering or higher bandwidth.

---

### Q2: What is the difference between Bandwidth, Throughput, and Latency? If an application is slow, how do you determine whether it is bandwidth-bound or latency-bound?
**Answer**:
- **Definitions**:
  - **Bandwidth**: The maximum theoretical data carrying capacity of the link (e.g., 100 Gbps).
  - **Latency**: The time taken for a bit/message to travel from sender to receiver (e.g., 50 ms RTT).
  - **Throughput**: The actual volume of useful data transferred per unit time.
- **Diagnostic Heuristic**:
  - **Latency-Bound Application**:
    - Characterized by many sequential, back-and-forth round trips (e.g., SQL queries inside an iterative loop, unpipelined HTTP/1.0 requests, synchronous RPCs).
    - Increasing link bandwidth from 100 Mbps to 10 Gbps produces **0% speedup**.
    - Reducing RTT (via CDN edge caching, connection pooling, batching) directly produces linear speedup.
  - **Bandwidth-Bound Application**:
    - Characterized by bulk streaming data transfers (e.g., video streaming, large database backups, S3 multi-part uploads).
    - Throughput approaches the link rate, and network interface utilization is near 100%. Increasing bandwidth directly reduces total transfer time.

---

### Q3: What is Jitter, why is it harmful, and how do real-time systems (like Zoom, WebRTC, VoIP) mitigate it?
**Answer**:
- **Definition**: Jitter is packet delay variation—the statistical difference in latency between successive packets arriving at the receiver.
- **Why It Is Harmful**: Real-time human speech and video require a continuous, evenly paced stream of audio frames (e.g., one 20 ms audio chunk every 20 ms). If packet 1 arrives in 30 ms, packet 2 in 90 ms, and packet 3 in 32 ms, playing them immediately upon arrival causes choppy, robotic audio distortion and video stutter.
- **Mitigation: Jitter Playout Buffers**:
  - The receiver delays rendering incoming packets by a calibrated buffer window (e.g. 50 ms).
  - Early-arriving packets are queued in memory; late-arriving packets arrive before their scheduled playout time.
  - Adaptive Jitter Buffers continuously estimate network jitter via RFC 3550 RTP timestamps and dynamically expand the buffer when jitter spikes and contract it during stable network conditions.
