# Ethernet, ARP, and Switching: Top FAANG Interview Questions

### Q1: What happens under the hood at Layer 2 when you ping `8.8.8.8` from your laptop?
**Answer**:
1. **Subnet Evaluation**: The laptop evaluates the destination IP `8.8.8.8` against its local subnet mask (e.g. `192.168.1.100/24`). Because `8.8.8.8` is not on the local subnet `192.168.1.0/24`, the OS kernel routes the packet to the **Default Gateway** (e.g. `192.168.1.1`).
2. **ARP Table Lookup**: The OS looks up the Default Gateway's IP (`192.168.1.1`) in its local ARP cache (`arp -a`).
   - If not cached: Laptop broadcasts an **ARP Request** (`FF:FF:FF:FF:FF:FF`): *"Who has 192.168.1.1? Tell 192.168.1.100"*.
   - The home router/gateway hears the broadcast and replies with a unicast **ARP Reply** containing its MAC address (`52:54:00:ab:cd:ef`).
   - Laptop records this mapping in its ARP cache.
3. **Frame Construction**: The laptop wraps the ICMP Echo Request IP packet into an Ethernet frame:
   - Destination MAC: Router's MAC (`52:54:00:ab:cd:ef`)
   - Source MAC: Laptop's Wi-Fi/Ethernet MAC
   - Destination IP: `8.8.8.8`
   - Source IP: `192.168.1.100`
4. **Router Forwarding**: The router receives the frame because the Destination MAC matches its interface. It strips the Ethernet header, decrements IPv4 TTL, performs NAT (rewriting Source IP to its public WAN IP), consults its routing table, and forwards the packet out its WAN fiber/cable interface.

---

### Q2: How does ARP Cache Poisoning work, and how do enterprise networks defend against it?
**Answer**:
- **Mechanism (The Flaw of ARP)**:
  - RFC 826 ARP is stateless and completely unauthenticated.
  - Most operating systems accept and update their ARP cache on receipt of an **unsolicited ARP reply** (Gratuitous ARP) without verifying if they ever asked for it.
  - An attacker sends forged ARP replies to Victim A: *"The Gateway 192.168.1.1 is at Attacker_MAC"*.
  - Simultaneously sends forged ARP replies to the Gateway: *"Victim 192.168.1.50 is at Attacker_MAC"*.
  - Both Victim and Gateway update their ARP tables. All outbound internet traffic from Victim flows through Attacker's machine first (Man-in-the-Middle). The attacker sniffs or tampers with unencrypted traffic and forwards it to the true gateway.
- **Enterprise Defenses**:
  1. **Dynamic ARP Inspection (DAI)**: Managed switches inspect ARP packets traversing switch ports. The switch validates ARP requests/replies against a trusted **DHCP Snooping binding database** (which tracks valid IP-to-MAC-to-Port mappings). Mismatched or spoofed ARP frames are dropped at the switch ASIC before reaching any hosts.
  2. **Static ARP Tables**: For critical database servers and gateways, static ARP entries (`arp -s`) prevent dynamic overwrites.
  3. **802.1X Port Authentication**: Requires cryptographic machine certificates before allowing any device to access the physical switch port.

---

### Q3: Why does a switch have a CAM table while a router has a Routing Table?
**Answer**:
| Characteristic | Switch CAM Table | Router Routing Table (FIB) |
| :--- | :--- | :--- |
| **Layer** | Layer 2 (Data Link) | Layer 3 (Network) |
| **Lookup Key** | Exact 48-bit MAC address (`AA:BB:CC:...`) | 32-bit IPv4 / 128-bit IPv6 prefix (`10.0.0.0/16`) |
| **Lookup Type** | **Exact Match** ($O(1)$ hash table or binary CAM) | **Longest Prefix Match (LPM)** (Trie / TCAM) |
| **Scope** | Local broadcast domain / LAN segment | Global hop-by-hop Internet |
| **Unknown Destination** | **Floods** frame out all ports | **Drops** packet (sends ICMP Destination Unreachable) or sends to Default Gateway (`0.0.0.0/0`) |
| **Population** | Autonomous dynamic **learning from Source MAC** of incoming frames | Dynamic routing protocols (BGP, OSPF) or static configuration |
