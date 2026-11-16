# Hands-On Exercises: Ethernet, ARP, and Switching

---

### Exercise 1: Tracing Ethernet Frame Traversal Across an L3 Router
**Goal**: Trace every Layer 2 MAC and Layer 3 IP header field as a packet traverses an intermediate router.

**Network Topology**:
- **Host A**: IP `192.168.1.50/24`, MAC `AA:AA:AA:AA:AA:AA`, Default Gateway `192.168.1.1`
- **Router R1**:
  - Interface `eth0` (facing Subnet 1): IP `192.168.1.1/24`, MAC `R1:11:11:11:11:11`
  - Interface `eth1` (facing Subnet 2): IP `10.0.0.1/24`, MAC `R1:22:22:22:22:22`
- **Host B**: IP `10.0.0.100/24`, MAC `BB:BB:BB:BB:BB:BB`, Default Gateway `10.0.0.1`

**Task**:
When Host A sends an HTTP request to Host B:
1. Does Host A send an ARP request for Host B's IP (`10.0.0.100`)? Why or why not?
2. What are the Source MAC, Destination MAC, Source IP, and Destination IP as the frame leaves Host A on Subnet 1?
3. What are the Source MAC, Destination MAC, Source IP, and Destination IP as the frame leaves Router R1 on Subnet 2?
4. What happens to the IPv4 TTL and Header Checksum fields at Router R1?

---

### Exercise 2: Broadcast Storm and Spanning Tree Protocol (STP)
**Goal**: Analyze loops in Layer 2 topologies.

**Scenario**:
Two switches (Switch 1 and Switch 2) are connected to each other with redundant redundant Ethernet cables (Cable A and Cable B) without Spanning Tree Protocol (STP) enabled.
1. Host 1 sends a single ARP request broadcast frame (`FF:FF:FF:FF:FF:FF`) to Switch 1.
2. Trace what happens on Cable A and Cable B in the next 5 switch cycles.
3. Why do Ethernet frames loop forever while IP packets do not? (Hint: Does Ethernet have a TTL?)
4. What is the observable impact on network switch CPU, link utilization, and host connectivity?
