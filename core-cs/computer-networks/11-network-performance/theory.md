# Theoretical Foundations: Network Performance & Optimization

---

## 1. Bandwidth-Delay Product (BDP) & The Long Fat Pipe

The **Bandwidth-Delay Product (BDP)** defines the capacity of the network "pipe"—the maximum volume of unacknowledged data that can be in flight simultaneously across the physical link:

$$\text{BDP (bits)} = \text{Bandwidth (bps)} \times \text{Round-Trip Time (seconds)}$$
$$\text{BDP (bytes)} = \frac{\text{Bandwidth} \times \text{RTT}}{8}$$

### The "Long Fat Network" (LFN) Problem:
Consider a modern 10 Gbps transatlantic link between New York and London ($\text{RTT} = 70 \text{ ms}$):
$$\text{BDP} = \frac{10 \times 10^9 \text{ bps} \times 0.070 \text{ s}}{8} = \mathbf{87.5 \text{ Megabytes}}$$

- If the client's TCP Receive Window (`rwnd`) is capped at the historical default of **64 KB** ($65,535 \text{ bytes}$):
  $$\text{Max Throughput} = \frac{65,535 \times 8 \text{ bits}}{0.070 \text{ s}} \approx 7,489,714 \text{ bps} \approx \mathbf{7.49 \text{ Mbps}}$$
- Even though the physical wire has 10,000 Mbps capacity, the application **only achieves 7.49 Mbps (< 0.1% utilization)**!
- **The Fix: RFC 7323 TCP Window Scale Option**:
  - Shifts the 16-bit window field left by up to 14 bits, scaling the maximum window size from $64 \text{ KB}$ up to **$1 \text{ Gigabyte}$**.
  - Modern Linux kernels automatically tune window sizes dynamically based on measured RTT.

---

## 2. Bufferbloat & Active Queue Management (AQM)

### The Problem: Drop-Tail Buffers
- Network equipment manufacturers historically addressed packet loss by adding massive RAM buffers (10 MB to 100 MB) into consumer and ISP routers.
- When an application executes bulk data transfers, TCP fills these buffers completely before packet drops occur.
- **Bufferbloat**: Packets sit in router queues for 500 ms to 2 seconds, turning a fast 15 ms fiber connection into an unusable, high-latency link. Interactive voice calls, online games, and DNS queries freeze.

### The Solution: CoDel (Controlled Delay - RFC 8289)
- Traditional AQM (RED - Random Early Detection) checked queue **size** (in bytes).
- CoDel tracks **packet sojourn time** (the actual time a packet spends waiting in the queue).
- **Core Algorithm**:
  - If minimum packet delay remains above a target threshold ($\text{TARGET} = 5 \text{ ms}$) for longer than an interval ($\text{INTERVAL} = 100 \text{ ms}$), CoDel declares the queue "persistently bloated".
  - Drops packets at a steadily increasing rate (or marks them via ECN - Explicit Congestion Notification).
  - This forces the sender's TCP stack to immediately back off before excessive queuing delay accumulates!

---

## 3. Content Delivery Network (CDN) Architecture

A CDN places caching reverse-proxy edge servers geographically close to end users in hundreds of Points of Presence (PoPs) worldwide:

```
[ User in Tokyo ] ------------ (150 ms RTT, 12,000 km) ------------> [ Origin Server in Virginia ]
                                                                                ^
                                                                                | Origin Fetch
                                                                                | (Once per hour)
[ User in Tokyo ] --- (2 ms RTT) ---> [ Cloudflare/CloudFront Edge in Tokyo ] -+
```

### Two Edge Steering Mechanisms:
1. **Anycast BGP Routing (Modern)**:
   - A single IP address (e.g. `1.1.1.1` or `104.16.0.1`) is announced to BGP routers by every CDN PoP globally.
   - BGP automatically routes each user's packet to the topologically nearest physical datacenter along the shortest AS path.
2. **GeoDNS Routing (Legacy/Alternative)**:
   - When resolving `cdn.example.com`, the authoritative DNS server inspects the resolver's IP address (via EDNS Client Subnet) and returns an IP address specific to the user's geographic region.

---

## 4. Key Linux Kernel Network Sysctl Tuning

For high-throughput 10G/40G/100G Linux server infrastructure:

```bash
# Maximum socket receive buffer (min, default, max in bytes)
sysctl -w net.ipv4.tcp_rmem="4096 87380 67108864"   # Max 64 MB

# Maximum socket send buffer
sysctl -w net.ipv4.tcp_wmem="4096 65536 67108864"   # Max 64 MB

# Enable modern TCP BBR congestion control
sysctl -w net.core.default_qdisc="fq"
sysctl -w net.ipv4.tcp_congestion_control="bbr"

# Max pending connections in kernel Accept Queue
sysctl -w net.core.somaxconn=4096

# Enable TCP Fast Open for client and server (RFC 7413)
sysctl -w net.ipv4.tcp_fastopen=3
```
