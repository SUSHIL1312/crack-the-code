# Common Mistakes in Ethernet, ARP, and Switching

### 1. Believing ARP Requests Cross Routers
- **Mistake**: Expecting Host A (`192.168.1.10`) to discover the MAC address of a server in AWS or Google (`142.250.190.46`) using ARP.
- **Reality**: ARP is strictly an unrouted Layer 2 link-layer broadcast protocol. Routers **never forward broadcast frames**. An ARP request sent by Host A is contained entirely within its local broadcast domain and terminated at the router interface. The destination MAC on an outgoing packet to Google is always the **local default router's MAC address**.

---

### 2. Confusing Collision Domains with Broadcast Domains
- **Mistake**: Thinking that replacing a Hub with an L2 Switch eliminates broadcast storms.
- **Reality**:
  - A switch splits **Collision Domains**: each connected port operates in dedicated, full-duplex transmission with zero collisions.
  - A switch **does NOT split Broadcast Domains**: any broadcast frame (`FF:FF:FF:FF:FF:FF`, like an ARP request or DHCP discovery) sent to one switch port is flooded out all connected ports across the entire switch. Only an L3 Router or an isolated 802.1Q VLAN can terminate a broadcast domain.

---

### 3. Ignoring MAC Table Exhaustion (CAM Flooding Attack)
- **Mistake**: Believing an enterprise switch will always maintain confidentiality between switch ports.
- **Reality**: Switch CAM tables have finite capacity (e.g., 8,000 to 128,000 MAC entries). If an attacker generates thousands of randomized bogus source MAC addresses per second, the switch's CAM table overflows. Once full, the switch enters **fail-open hub mode (fail-safe)**, flooding all subsequent unicast traffic out every port, allowing the attacker to sniff private conversations!
- **Fix**: Enable **Port Security** on the switch to limit the number of learned MAC addresses per physical port (e.g. max 2 MACs) and shut down the port on violation.

---

### 4. Overlooking Gratuitous ARP Updates in Cloud Failover
- **Mistake**: Migrating a secondary database to become the primary with a new Virtual IP, without broadcasting a Gratuitous ARP (GARP).
- **Reality**: Upstream switches retain the old primary's MAC address in their CAM tables for up to 300 seconds (5 minutes). Without an immediate GARP broadcast to flush and refresh switch CAM tables, traffic will continue flowing to the dead primary for several minutes after failover.
