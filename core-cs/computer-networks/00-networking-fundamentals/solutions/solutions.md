# Solutions: Networking Fundamentals Exercises

---

### Solution 1: GEO vs LEO Satellite Latency

1. **GEO Satellite**:
   - Distance up and down: $2 \times 35,786 \text{ km} = 71,572 \text{ km}$.
   - One-way propagation delay:
     $$D_{\text{prop, one-way}} = \frac{71,572 \text{ km}}{300,000 \text{ km/s}} \approx 0.2386 \text{ s} \approx 238.6 \text{ ms}$$
   - Round-Trip Time (RTT):
     $$\text{RTT} = 2 \times 238.6 \text{ ms} \approx 477.2 \text{ ms}$$
   - When factoring in atmospheric transit, ground station processing, and terrestrial routing, real-world GEO RTT is $\approx 600 - 700 \text{ ms}$.

2. **LEO Satellite (Starlink)**:
   - Distance up and down: $2 \times 550 \text{ km} = 1,100 \text{ km}$.
   - One-way propagation delay:
     $$D_{\text{prop, one-way}} = \frac{1,100 \text{ km}}{300,000 \text{ km/s}} \approx 3.67 \text{ ms}$$
   - Minimum theoretical RTT:
     $$\text{RTT} = 2 \times 3.67 \text{ ms} \approx 7.33 \text{ ms}$$
   - Real-world LEO RTT averages $\approx 25 - 40 \text{ ms}$, which is competitive with terrestrial fiber.

3. **Conclusion**:
   - GEO's half-second (~600 ms) delay is fundamentally unacceptable for real-time interactions, as human perception detects lag above ~100 ms, and TCP handshakes require several RTTs.

---

### Solution 2: Store-and-Forward Pipelining

- Link rate $R = 10 \text{ Mbps} = 10^7 \text{ bps}$.
- 3 links $\implies 2$ intermediate routers.
- Propagation delay per link:
  $$D_{\text{prop}} = \frac{1,000 \text{ km}}{200,000 \text{ km/s}} = 5 \text{ ms}$$
  Total propagation across 3 links: $3 \times 5 \text{ ms} = 15 \text{ ms}$.
- Total processing across 2 routers: $2 \times 1 \text{ ms} = 2 \text{ ms}$.

#### Strategy A: Single 10 MB Packet ($8 \times 10^7 \text{ bits}$)
- Transmission delay per hop:
  $$D_{\text{trans}} = \frac{80,000,000 \text{ bits}}{10,000,000 \text{ bps}} = 8 \text{ seconds}$$
- Because routers are **store-and-forward**, Router 1 must receive the **entire** 10 MB packet before transmitting it to Router 2.
- Total time:
  $$\text{Time} = 3 \times D_{\text{trans}} + \text{Processing} + \text{Propagation} = 3 \times 8 \text{ s} + 0.002 \text{ s} + 0.015 \text{ s} = \mathbf{24.017 \text{ seconds}}$$

#### Strategy B: Segmented Pipelining ($10,000$ packets of $1 \text{ KB} = 8,000 \text{ bits}$)
- Transmission delay per small packet:
  $$d_{\text{trans}} = \frac{8,000}{10^7} = 0.0008 \text{ s} = 0.8 \text{ ms}$$
- Pipelining: As soon as packet 1 is transmitted by Host A to Router 1, Host A immediately starts transmitting packet 2 while Router 1 forwards packet 1!
- The total time is the time for the first packet to reach the destination ($3 \times 0.8 \text{ ms} + 2 \text{ ms} + 15 \text{ ms} = 19.4 \text{ ms}$) plus the time for the remaining $9,999$ packets to emerge from the pipeline at rate $d_{\text{trans}}$:
  $$\text{Time} = 19.4 \text{ ms} + (9,999 \times 0.8 \text{ ms}) = 19.4 \text{ ms} + 7,999.2 \text{ ms} \approx \mathbf{8.0186 \text{ seconds}}$$
- **Result**: Pipelining via packet switching is **3x faster** ($8 \text{ s}$ vs $24 \text{ s}$) because links are utilized concurrently!

---

### Solution 3: BDP and TCP Window Limitation

1. **Bandwidth-Delay Product**:
   $$\text{BDP} = 40 \text{ Gbps} \times 0.070 \text{ s} = 2.8 \text{ Gigabits} = \frac{2.8 \times 10^9}{8 \times 10^6 \text{ bytes}} = \mathbf{350 \text{ Megabytes}}$$
2. **Achievable Throughput with 64 KB Window**:
   $$\text{Max Throughput} = \frac{\text{Window Size}}{\text{RTT}} = \frac{64 \times 1024 \times 8 \text{ bits}}{0.070 \text{ s}} \approx 7,489,828 \text{ bps} \approx \mathbf{7.49 \text{ Mbps}}$$
3. **Percentage of Link Wasted**:
   $$\text{Utilization} = \frac{7.49 \text{ Mbps}}{40,000 \text{ Mbps}} \approx 0.0187\%$$
   $$\mathbf{99.98\%} \text{ of the 40 Gbps link is completely wasted!}$$
   To saturate the 40 Gbps link, RFC 7323 TCP Window Scaling must be enabled with a window size of at least $350 \text{ MB}$.
