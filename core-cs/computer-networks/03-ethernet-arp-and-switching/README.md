# Module 03: Ethernet, ARP, and Switching

## Overview
While the Network Layer (IP) is responsible for end-to-end routing across the global Internet, data can only physically move from one node to the next across local links using Data Link Layer (Layer 2) protocols. Ethernet is the ubiquitous standard for wired local area networks (LANs) and datacenter fabrics.

In this module, you will master the mechanics of Ethernet framing, 48-bit MAC addressing, how the Address Resolution Protocol (ARP) glues Layer 3 IP to Layer 2 MAC addresses, and how L2 switches build CAM tables to forward frames without collisions.

---

## Learning Objectives
1. **Ethernet Framing**: Dissect the IEEE 802.3 and Ethernet II frame formats, understanding minimum frame size (64 bytes), MTU (1500 bytes), and CRC-32 Frame Check Sequence.
2. **MAC Addressing**: Master the 48-bit hardware address format, Organizationally Unique Identifiers (OUIs), and Unicast vs Multicast vs Broadcast bitflags.
3. **Address Resolution Protocol (ARP)**:
   - Trace ARP Requests (Layer 2 broadcast `FF:FF:FF:FF:FF:FF`) and ARP Replies (Layer 2 unicast).
   - Understand Gratuitous ARP (GARP) in High-Availability failovers (Keepalived, VRRP).
   - Analyze ARP Cache Poisoning (Man-in-the-Middle) and defense mechanisms (Dynamic ARP Inspection).
4. **Switching & CAM Tables**: Learn how switches perform transparent bridging via source MAC learning, forward filtering, and unknown unicast flooding.
5. **Collision vs Broadcast Domains**: Understand how switches segment collision domains and how VLANs (IEEE 802.1Q) isolate broadcast domains.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of Ethernet CSMA/CD history, full-duplex switching, ARP protocol, and VLANs.
- `internal-working.md`: Switch Content Addressable Memory (CAM) tables, ARP cache kernel state machines, and 802.1Q tag insertion.
- `visual-explanation.md`: Step-by-step visual walk of ARP request/reply flow and switch MAC learning tables.
- `examples/`:
  - `arp_table_simulator.cpp`: Complete C++ simulator of an L2 switch CAM table and host ARP cache with attack detection.
- `exercises/`: Switch forwarding trace problems and ARP analysis exercises.
- `solutions/`: Reference walkthroughs and answers.
- `interview-questions.md`: FAANG interview questions (e.g. "What happens when Host A sends a packet to Host B on a different subnet?", "ARP spoofing prevention").
- `common-mistakes.md`: Broadcast storms, assuming ARP crosses routers, confusing MAC learning with routing.
