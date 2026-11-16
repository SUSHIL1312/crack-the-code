# IP Addressing & Subnetting: Top FAANG Interview Questions

### Q1: What is Longest Prefix Match (LPM), and how do high-throughput routers perform it in hardware at 400 Gbps line rate?
**Answer**:
- **Concept**: A router's routing table (FIB) may contain multiple overlapping routes matching a destination IP (e.g. `10.0.0.0/8`, `10.1.0.0/16`, `10.1.2.0/24`). The router must select the route with the longest prefix (most specific match).
- **The Challenge at 400 Gbps**: A 400 Gbps link can receive up to $\approx 595 \text{ million packets per second}$ (minimum 64-byte packets). The router has only $\approx 1.68 \text{ nanoseconds}$ per packet to determine the egress port across a table of 1,000,000+ routes.
- **Hardware Solution: TCAM (Ternary Content Addressable Memory)**:
  - Unlike standard RAM which takes an address and returns data, CAM takes data (the destination IP) and returns the address/index where that data is stored in $O(1)$ time.
  - **Ternary CAM**: Allows a third state "Don't Care" (mask bit `X`) alongside `0` and `1`.
  - Routes are stored in TCAM sorted in descending order of prefix length (longest prefixes first).
  - When a packet arrives, the 32-bit IP is broadcast across all TCAM cells simultaneously. All matching entries fire, and a hardware priority encoder selects the first match (guaranteed to be the longest prefix) in a single clock cycle!

---

### Q2: Why did NAT (Network Address Translation) delay the worldwide adoption of IPv6 by decades? What are the architectural drawbacks of NAT?
**Answer**:
1. **How NAT Saved IPv4**:
   - RFC 1631 / RFC 3022 NAT (and NAPT/PAT - Port Address Translation) allowed thousands of internal machines with RFC 1918 private IPs (`10.0.0.0/8`, `192.168.0.0/16`) to share a single public IPv4 address.
   - By multiplexing outbound connections over distinct Layer 4 source ports ($65,535$ ports per public IP), organizations avoided purchasing millions of public IPs, postponing IPv4 exhaustion from 1996 to the 2010s.
2. **Architectural Drawbacks of NAT**:
   - **Violates End-to-End Principle**: Hosts behind NAT cannot be directly addressed from the outside Internet, breaking peer-to-peer applications (VoIP, WebRTC, BitTorrent) and requiring complex traversal protocols (STUN, TURN, ICE, UPnP).
   - **Stateful Middlebox Failure**: NAT gateways must maintain per-connection state tables in RAM. If the NAT gateway crashes or reboots, all active TCP connections traversing it are destroyed.
   - **Layer Violation**: NAT must rewrite Layer 3 IP headers and Layer 4 TCP/UDP checksums, and inspect application payloads for embedded IPs (e.g., FTP `PORT` commands, SIP), breaking end-to-end cryptographic integrity (IPsec AH).

---

### Q3: In AWS VPC, why can't you use the first 4 and last 1 IP addresses in any subnet?
**Answer**:
In every AWS VPC subnet, AWS reserves **5 IP addresses** automatically:
- For example, in subnet `10.0.0.0/24`:
  1. `10.0.0.0`: **Network Address** (reserved by standard IP networking).
  2. `10.0.0.1`: **VPC Router** (default gateway for the subnet).
  3. `10.0.0.2`: **Amazon Provided DNS / Route 53 Resolver** (Subnet base + 2).
  4. `10.0.0.3`: **Reserved by AWS for future use**.
  5. `10.0.0.255`: **Network Broadcast Address** (AWS does not support broadcast in VPC, but reserves the address per standard RFC).
- **Result**: In a `/28` subnet ($16$ total IPs), only $16 - 5 = \mathbf{11 \text{ usable IP addresses}}$ are available for EC2 instances or container ENIs.
