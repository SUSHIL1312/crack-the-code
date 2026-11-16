# Common Mistakes in Networking Fundamentals

### 1. Believing Upgrading Bandwidth Always Decreases Latency
- **Mistake**: A team upgrades their server connection from 1 Gbps to 10 Gbps and expects their inter-region REST API latency from California to Frankfurt to drop.
- **Reality**: Propagation delay ($D_{\text{prop}} = d / s$) accounts for 99% of wide-area network (WAN) latency. Increasing bandwidth only shrinks transmission delay ($D_{\text{trans}} = L / R$), which for a 1 KB HTTP request is already less than 10 microseconds! Total RTT remains virtually identical (~140 ms).

---

### 2. Confusing Throughput with Goodput
- **Mistake**: Measuring network throughput by counting raw bytes traversing the network card (`eth0`) and assuming the application is running at that speed.
- **Reality**: Raw bytes include Ethernet headers (14 bytes), IP headers (20 bytes), TCP headers (20-60 bytes), TLS cryptographic overhead, and most importantly, **retransmitted packets** caused by packet loss. If a network suffers 20% packet loss, throughput may look high, but goodput (useful payload) collapses.

---

### 3. Ignoring Queuing Delay in Capacity Planning
- **Mistake**: Calculating latency using only propagation delay ($d / s$) and transmission delay ($L / R$).
- **Reality**: Under heavy network load (traffic intensity $I = \frac{L \cdot a}{R} \to 1$), queuing delay grows asymptotically towards infinity. A link operating at 95% utilization will experience severe latency spikes, bufferbloat, and random tail drops.

---

### 4. Overlooking Path Asymmetry in the Internet
- **Mistake**: Assuming that packet paths from Host A to Host B and from Host B to Host A are identical and have equal latency.
- **Reality**: The Internet uses **hot-potato routing** across autonomous systems (BGP). Traffic from A to B often traverses an entirely different sequence of ISPs, fiber cables, and geographic routes than traffic returning from B to A (asymmetric routing).
