# Theoretical Foundations: Ethernet, ARP, and Switching

---

## 1. Ethernet Framing Architecture (Ethernet II)

Modern Ethernet frames follow the Ethernet II (DIX) standard:

| Field | Size | Description |
| :--- | :--- | :--- |
| **Preamble** | 7 Bytes | Alternating pattern `10101010` used by physical receiver PHY for clock synchronization. |
| **Start Frame Delimiter (SFD)** | 1 Byte | Byte `10101011` indicating that destination MAC begins on the next byte. |
| **Destination MAC** | 6 Bytes | 48-bit MAC address of the receiver interface on the local link. |
| **Source MAC** | 6 Bytes | 48-bit MAC address of the sending network interface. |
| **EtherType** | 2 Bytes | Identifies upper-layer protocol (`0x0800` IPv4, `0x0806` ARP, `0x86DD` IPv6, `0x8100` 802.1Q VLAN). |
| **Payload** | 46 to 1500 Bytes | Encapsulated packet (e.g. IP packet). Padded with zeroes if $< 46 \text{ bytes}$. |
| **Frame Check Sequence (FCS)** | 4 Bytes | CRC-32 cyclic redundancy check for detecting transmission bit errors. |

### Minimum and Maximum Frame Sizes:
- **Minimum Size**: $6 + 6 + 2 + 46 + 4 = \mathbf{64 \text{ bytes}}$ (required historically by CSMA/CD to detect collisions before frame transmission finished).
- **Maximum Size (Standard MTU)**: $6 + 6 + 2 + 1500 + 4 = \mathbf{1518 \text{ bytes}}$ (or $1522 \text{ bytes}$ with 4-byte 802.1Q VLAN tag).
- **Jumbo Frames**: Datacenters often support MTU of 9,000 bytes ($9018 \text{ bytes}$ frame) to reduce CPU interrupt overhead by 6x.

---

## 2. MAC Addressing (48 Bits)

A Media Access Control (MAC) address is physically burned into the network interface card (NIC) ROM (also known as a Burned-In Address, BIA).

$$\underbrace{\text{52 : 54 : 00}}_{\text{OUI (24 bits)}} \ : \ \underbrace{\text{12 : 34 : 56}}_{\text{Device NIC ID (24 bits)}}$$

- **OUI (Organizationally Unique Identifier)**: Assigned by IEEE to hardware manufacturers (e.g., Apple, Cisco, Intel, Realtek).
- **Individual/Group (I/G) Bit**: Bit 0 of the first octet:
  - `0`: Unicast address (identifies a single NIC).
  - `1`: Multicast address (e.g. IPv4 multicast `01:00:5E:...`, or broadcast `FF:FF:FF:FF:FF:FF`).
- **Universally / Locally Administered (U/L) Bit**: Bit 1 of the first octet:
  - `0`: Universally unique (burned-in by vendor).
  - `1`: Locally administered (assigned by software, hypervisors, containers).

---

## 3. Address Resolution Protocol (ARP - RFC 826)

IP addresses are logical abstractions. A network switch or NIC cannot deliver a packet using an IP address; it requires the physical 48-bit destination MAC address.

### The ARP Workflow:
When Host A (`192.168.1.10`) wants to send an IP packet to Host B (`192.168.1.20`) on the same subnet:
1. **ARP Cache Lookup**: Host A checks its local ARP cache table.
   - If present: Immediately wraps the IP packet into an Ethernet frame with Host B's MAC address.
2. **ARP Request Broadcast**:
   - Host A generates an ARP Request: *"Who has IP 192.168.1.20? Tell 192.168.1.10"*.
   - Destination MAC is set to **`FF:FF:FF:FF:FF:FF` (Broadcast)**.
   - Every switch in the broadcast domain floods the frame out all ports.
3. **ARP Reply Unicast**:
   - Every host receives the broadcast. All hosts except Host B inspect the target IP, realize it is not them, and silently drop the frame.
   - Host B updates its own ARP table with Host A's IP-to-MAC mapping.
   - Host B sends a direct **unicast ARP Reply** to Host A's MAC: *"192.168.1.20 is at MAC 52:54:00:12:34:56"*.
4. **Transmission**: Host A caches the entry and transmits the pending IP packet.

### Gratuitous ARP (GARP):
An ARP announcement where a host sends an ARP request or reply with its own IP as both source and target IP.
- Used on boot to detect if another machine already has the same IP (IP address conflict detection).
- Used during High Availability failover (e.g., Keepalived / VRRP): when the primary database crashes, the backup node takes over the Virtual IP (VIP) and immediately broadcasts a GARP. This updates all network switches' CAM tables to route traffic to the backup node's MAC address in milliseconds!

---

## 4. Ethernet Switching & CAM Tables

Unlike legacy Ethernet Hubs (which repeated every bit to all ports, creating a single shared collision domain), an **L2 Switch** operates at the Data Link layer and provides dedicated full-duplex bandwidth per port.

### The Learning & Forwarding Algorithm:
A switch maintains a **CAM Table (Content Addressable Memory)** mapping `MAC Address -> (Egress Port, Age)`:
1. **Source Learning**: When a frame arrives on Port $P$ with Source MAC $S$:
   - The switch adds or refreshes the entry: `MAC S is on Port P`.
2. **Forwarding Decision**:
   - If Destination MAC $D$ is **Broadcast (`FF:FF:FF:FF:FF:FF`)**: Flood out all ports except ingress port $P$.
   - If Destination MAC $D$ is known in the CAM table for Port $Q$:
     - If $Q == P$: Drop (filtered, destination is on the same segment).
     - If $Q \neq P$: Forward the frame strictly out Port $Q$ (Unicast forwarding).
   - If Destination MAC $D$ is unknown: **Unknown Unicast Flooding** (flood out all ports except $P$ until destination replies).
3. **Aging**: If no frames are seen from a MAC for 300 seconds (default aging time), the entry is pruned from the CAM table.
