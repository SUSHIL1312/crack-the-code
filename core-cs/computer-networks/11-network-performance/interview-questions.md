# FAANG Interview Questions: Network Performance & Optimization

---

### Question 1: What is the Bandwidth-Delay Product (BDP), and why does an under-provisioned TCP window cap throughput regardless of physical link bandwidth?
**Target Companies**: Google, Netflix, Meta, AWS.

#### Answer:
The **Bandwidth-Delay Product (BDP)** defines the maximum quantity of unacknowledged data that can be in-flight in a network pipe at any given instant before the pipe is completely saturated:
$$\text{BDP} = \text{Bandwidth} \times \text{Round-Trip Time (RTT)}$$

In TCP, data transmission is strictly flow-controlled by the **Effective Window**:
$$W = \min(\text{cwnd}, \text{rwnd})$$
where $\text{cwnd}$ is the sender's congestion window and $\text{rwnd}$ is the receiver's advertised receive window.

If the buffer space allocated to the socket window ($W$) is less than the physical BDP:
$$\text{Max Achievable Throughput} = \frac{W}{\text{RTT}}$$
- **Mechanism**: The sender transmits $W$ bytes. Once $W$ bytes are in flight, the sender's transmit buffer is empty, and the TCP protocol prohibits transmitting any more bytes until the receiver returns an ACK.
- During the remainder of the RTT ($\text{RTT} - \frac{W}{\text{Bandwidth}}$), the physical fiber link sits completely idle.
- **Example**: On a 10 Gbps transatlantic link with 100 ms RTT, the BDP is:
  $$\text{BDP} = 10 \times 10^9\text{ bps} \times 0.1\text{ s} = 1\times 10^9\text{ bits} = 125\text{ MB}$$
  If the receiver's TCP window is capped at the legacy 16-bit ceiling ($65{,}535\text{ bytes} \approx 64\text{ KB}$):
  $$\text{Throughput} = \frac{65{,}535\text{ bytes} \times 8\text{ bits/byte}}{0.1\text{ s}} \approx 5.24\text{ Mbps}$$
  The application utilizes only $0.05\%$ of the 10 Gbps line! RFC 7323 TCP Window Scale Option (`SO_RCVBUF` / `tcp_rmem` auto-tuning up to 1 GB) is required to fill the pipe.

---

### Question 2: Explain "Bufferbloat". How does CoDel (Controlled Delay) solve it differently than classic Drop-Tail or Random Early Detection (RED)?
**Target Companies**: Apple, Google, Cloudflare, Qualcomm.

#### Answer:
- **Bufferbloat Definition**: Excessively large network router buffers that queue up gigabytes of packets. Instead of dropping packets when capacity is reached, oversized FIFO buffers delay packets for hundreds or thousands of milliseconds. Because classic TCP Reno/Cubic only senses congestion upon packet loss, TCP continues ramping up send rates until packets spend huge amounts of time sitting in FIFO queues, destroying interactive latency (gaming, video calls, VoIP, SSH) while providing negligible throughput gain.

- **Classic Drop-Tail**:
  - Waits until the buffer is 100% full, then abruptly drops all subsequent incoming packets.
  - Causes **Global Synchronization**: Multiple concurrent TCP flows all experience packet loss simultaneously, simultaneously halve their congestion windows, simultaneously recover, and oscillate wildly between starvation and buffer overflow.

- **Random Early Detection (RED)**:
  - Tracks average queue length. Once average queue length crosses a minimum threshold, it starts dropping packets probabilistically before the buffer becomes completely full.
  - **Weakness**: Requires complex manual tuning of min/max thresholds and weight parameters; performs poorly under variable bandwidth and dynamic traffic loads.

- **CoDel (Controlled Delay - RFC 8290)**:
  - Instead of inspecting buffer size in **bytes or packets**, CoDel tracks the **sojourn time** (standing queue delay) of packets traversing the queue:
    $$\text{Sojourn Time} = t_{\text{dequeue}} - t_{\text{enqueue}}$$
  - CoDel distinguishes between "good queue" (transient bursts absorbing short spikes) and "bad queue" (persistent standing queues that never drain).
  - If the minimum observed sojourn time over a sliding window (typically 100 ms) remains above a target threshold (typically 5 ms), CoDel recognizes a persistent bottleneck and initiates deterministic packet drops at exponentially increasing intervals until the queue drains below target.

---

### Question 3: How do CDNs route user requests to the optimal Point of Presence (PoP)? Compare Anycast BGP Routing vs GeoDNS Routing.
**Target Companies**: Meta, Cloudflare, Fastly, Netflix.

#### Answer:

| Feature | Anycast BGP Routing | GeoDNS Routing |
| :--- | :--- | :--- |
| **Routing Mechanism** | Multiple edge data centers announce the **identical IP address** via Border Gateway Protocol (BGP). Internet routers automatically route packets to the topologically closest PoP via shortest AS path. | Authoritative DNS nameserver inspects client IP (or EDNS Client Subnet) and returns an IP address geographically closest to the user. |
| **DDoS Resilience** | Exceptional. Malicious volumetric attacks are automatically absorbed and distributed across all global PoPs. | Poor. DNS caching prevents rapid shifting of attack traffic, and authoritative DNS servers can themselves be overwhelmed. |
| **Failover Speed** | Instantaneous. BGP route withdrawal immediately redirects subsequent packets to the next closest surviving PoP. | Slow. Limited by DNS TTL caching (often 30–300 seconds); client resolvers frequently violate TTLs and retain stale IPs. |
| **TCP Route Flapping** | Risky. Mid-connection BGP route recalculations can cause packets belonging to an active TCP stream to land on a different edge server, resetting connections (mitigated by BGP stability tuning or internal tunneling). | Safe. Once resolved, the client communicates with a specific unicast IP for the lifetime of the connection. |
| **Implementation Complexity** | High. Requires owning Autonomous System Number (ASN) and public BGP peering arrangements. | Low. Can be configured via managed DNS providers (e.g. Route 53, Cloudflare DNS). |

---

### Question 4: How does TCP BBR (Bottleneck Bandwidth and RTT) differ fundamentally from loss-based congestion control algorithms like Cubic?
**Target Companies**: Google, YouTube, Meta.

#### Answer:
- **Loss-based algorithms (Reno, Cubic)** conflate packet loss with congestion:
  - They ramp up `cwnd` monotonically until packet loss occurs (buffer overflow), then abruptly drop `cwnd`.
  - On modern networks with lossy wireless (WiFi/5G) or bloated intermediate buffers, loss-based algorithms either throttle prematurely due to non-congestion packet loss or fill bloated buffers, maximizing latency.
- **BBR (Bottleneck Bandwidth and RTT)**:
  - BBR models the physical network by simultaneously tracking two independent physical properties:
    1. Maximum delivery rate (bottleneck bandwidth, $\text{BtlBw}$).
    2. Minimum round-trip time (propagation delay, $\text{RTprop}$).
  - BBR operates at the optimal operating point (Kleinrock's optimal operating point):
    $$\text{In-Flight Target} = \text{BtlBw} \times \text{RTprop}$$
  - It paces packet transmissions at exactly $\text{BtlBw}$, keeping the bottleneck pipe full while keeping intermediate queues completely empty (zero bufferbloat).
  - BBR ignores sporadic packet loss up to 15–20%, making it dramatically superior on satellite, cellular, and cross-continental fiber links.
