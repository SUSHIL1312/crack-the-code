# Internal Working: Packet Forwarding & Physical Media Latency

---

## 1. Inside a Modern Network Router / Switch

How does a packet traverse a high-speed router (such as a Cisco/Arista 400 Gbps datacenter switch)?

```
Physical Link (Fiber)
       |
       v
+-------------------------------------------------------------+
| Physical Layer (PHY) & MAC Controller                       |
|   - Converts optical photons/analog signals to digital bits |
|   - Verifies Ethernet CRC checksum                          |
+-------------------------------------------------------------+
       |
       v
+-------------------------------------------------------------+
| Ingress Network Processing Unit (NPU / ASIC)                |
|   - Extracts IP header (Source IP, Destination IP, TTL)     |
|   - Decrements TTL (drops packet if TTL == 0 with ICMP)     |
|   - Recomputes IPv4 header checksum                         |
|   - Lookups Destination IP in TCAM (Ternary Content         |
|     Addressable Memory) in parallel: 1 clock cycle lookup   |
+-------------------------------------------------------------+
       |
       v
+-------------------------------------------------------------+
| Switch Fabric (Crossbar Matrix)                             |
|   - Routes packet from ingress port to target egress port   |
+-------------------------------------------------------------+
       |
       v
+-------------------------------------------------------------+
| Egress Queue Buffer (Packet Memory)                         |
|   - FIFO / Fair-Queuing / Weighted Random Early Detection   |
|   - If buffer is full -> Tail Drop (Packet Lost)            |
+-------------------------------------------------------------+
       |
       v
Physical Link Transmit (Serializer / Deserializer - SerDes)
```

1. **Hardware Forwarding (Fast Path)**: 99.99% of packets are forwarded purely by hardware ASICs with latencies under 500 nanoseconds (Cut-Through Switching).
2. **Software Control Plane (Slow Path)**: Packets requiring complex handling (e.g. BGP routing updates, ARP requests for unknown IPs, ICMP TTL exceeded) are redirected to the router's general-purpose CPU.

---

## 2. Speed of Light in Fiber: The Hard Floor of Internet Latency

Engineers often ask: *"Can better software or quantum computing make London-to-New-York latency 1 millisecond?"*

The laws of physics dictate the absolute theoretical minimum:
1. Great-circle distance from New York to London: $\approx 5,585 \text{ km}$.
2. Refractive index of standard single-mode optical fiber ($SiO_2$ glass):
   $$n \approx 1.468$$
3. Speed of light in fiber:
   $$v_{\text{fiber}} = \frac{c}{n} = \frac{299,792 \text{ km/s}}{1.468} \approx 204,200 \text{ km/s} \approx 204.2 \text{ km/ms}$$
4. One-way propagation delay in a direct ideal fiber cable:
   $$D_{\text{prop}} = \frac{5,585 \text{ km}}{204.2 \text{ km/ms}} \approx 27.35 \text{ ms}$$
5. Minimum theoretical Round-Trip Time (RTT):
   $$\text{RTT}_{\text{min}} = 2 \times 27.35 \text{ ms} \approx 54.7 \text{ ms}$$
6. Real-world RTT: Real submarine cables cannot follow a straight line (they curve along ocean shelves, traverse landing stations, and pass through repeaters/optical amplifiers), making actual measured NYC-to-London RTT approximately **65 to 75 ms**.

No amount of bandwidth (even 1,000 Terabits/sec) can reduce this RTT below the speed of light in glass.
