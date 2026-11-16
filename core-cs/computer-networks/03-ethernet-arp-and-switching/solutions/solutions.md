# Solutions: Ethernet, ARP, and Switching Exercises

---

### Solution 1: Packet Traversal Across Router

1. **Does Host A send an ARP request for Host B (`10.0.0.100`)?**
   - **No!** Host A performs a bitwise AND between destination IP `10.0.0.100` and its subnet mask `255.255.255.0`.
   - Result: `10.0.0.0` $\neq$ `192.168.1.0` (Host A's local subnet).
   - Host A recognizes that Host B is on a **remote network**.
   - Host A queries its routing table for the Default Gateway (`192.168.1.1`).
   - Host A issues an ARP request for the **Default Gateway's MAC address (`R1:11:11:11:11:11`)**, NOT Host B's MAC address!

2. **Frame Leaving Host A (Subnet 1)**:
   - Source MAC: `AA:AA:AA:AA:AA:AA`
   - Destination MAC: `R1:11:11:11:11:11` (Default Gateway)
   - Source IP: `192.168.1.50` (Host A)
   - Destination IP: `10.0.0.100` (Host B)

3. **Frame Leaving Router R1 (Subnet 2)**:
   - Router R1 strips the incoming Ethernet header.
   - Router R1 resolves Host B's IP on Subnet 2 via ARP (or looks up its existing ARP table).
   - Router R1 encapsulates the IP packet into a **new** Ethernet frame:
     - Source MAC: `R1:22:22:22:22:22` (Router R1's `eth1` egress MAC)
     - Destination MAC: `BB:BB:BB:BB:BB:BB` (Host B's MAC)
     - Source IP: `192.168.1.50` (Unchanged!)
     - Destination IP: `10.0.0.100` (Unchanged!)

4. **Modifications inside the IPv4 Header**:
   - `TTL`: Decremented by 1 (`TTL = TTL - 1`). If TTL reaches 0, router drops the packet and sends `ICMP Time Exceeded`.
   - `Header Checksum`: Recalculated because the TTL field changed.

---

### Solution 2: Broadcast Storm and Loop Dynamics

1. **Cycle Trace**:
   - Cycle 1: Host 1 sends broadcast `F` to Switch 1.
   - Cycle 2: Switch 1 floods `F` over both Cable A and Cable B to Switch 2.
   - Cycle 3: Switch 2 receives `F` from Cable A and floods it out Cable B (back to Switch 1). Simultaneously, Switch 2 receives `F` from Cable B and floods it out Cable A (back to Switch 1).
   - Cycle 4: Switch 1 receives two duplicated copies of `F`, one on Cable A and one on Cable B. Switch 1 floods each copy back across the other cable.
   - Cycle 5: Packets multiply exponentially ($2, 4, 8, 16...$).

2. **Why Ethernet Frames Loop Forever**:
   - **IPv4 has a TTL (Time to Live) field** (and IPv6 has Hop Limit). If an IP packet loops, the TTL eventually decrements to 0 and the router drops it.
   - **Ethernet II frames have NO TTL field!** Once a loop exists in a Layer 2 network, broadcast frames circulate infinitely until a physical cable is unplugged or hardware crashes.

3. **Observable Impact**:
   - **Switch CPU Saturation**: Switch ASICs and control plane processors reach 100% CPU processing millions of looped frames per second.
   - **CAM Table Thrashing**: The switch continuously updates its CAM table because the same Source MAC arrives alternately on Port A and Port B hundreds of times per second (MAC flapping).
   - **Total Network Outage**: 100% link bandwidth saturation; all legitimate unicast traffic is starved, and connected hosts freeze responding to network interrupts.
   - **Fix**: Spanning Tree Protocol (STP / IEEE 802.1D / RSTP 802.1w) automatically blocks redundant ports logically, breaking the loop while maintaining failover redundancy.
