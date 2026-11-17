# Module 02: IP Addressing and Subnetting

<!-- Chapter Navigation Header -->
| ⬅️ [Module 01: OSI & TCP/IP Model](../01-osi-and-tcp-ip-model/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Module 03: Ethernet, ARP & Switching](../03-ethernet-arp-and-switching/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## Overview
The Internet Protocol (IP) provides logical addressing and routing across heterogeneous physical networks. Understanding IPv4 subnetting, bitwise address calculations, Classless Inter-Domain Routing (CIDR), RFC 1918 private address ranges, and the transition to IPv6 (128-bit addresses) is an essential prerequisite for cloud systems architecture, VPC configuration (AWS/GCP/Azure), and networking interviews.

In this module, you will master the mathematical algorithms behind CIDR masks, subnets, and host range allocations, and implement a high-performance CIDR subnet calculator in Modern C++.

---

## Learning Objectives
1. **IPv4 Binary Structure**: Master the 32-bit integer representation of IPv4 addresses and dotted-decimal formatting.
2. **Subnet Math & Bitwise Logic**:
   - Network Address: $\text{IP} \land \text{Mask}$ (`IP & Mask`)
   - Broadcast Address: $\text{IP} \mid (\sim \text{Mask})$
   - Total Addresses: $2^{32 - \text{prefix}}$
   - Usable Host Capacity: $2^{32 - \text{prefix}} - 2$
3. **Classful vs CIDR (RFC 1519)**: Understand how CIDR prevented the collapse of global routing tables in the mid-1990s through route aggregation (supernetting).
4. **Special Address Blocks**: RFC 1918 Private networks, RFC 3927 APIPA (`169.254.0.0/16`), RFC 3021 point-to-point links (`/31`), and Loopback (`127.0.0.0/8`).
5. **IPv6 Architecture**: 128-bit addressing, zero compression (`::`), SLAAC (Stateless Address Autoconfiguration), and address scopes (Link-Local `fe80::/10`, Global Unicast `2000::/3`).

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of IP routing, address exhaustion, NAT, CIDR, and IPv6.
- `internal-working.md`: Bitwise arithmetic algorithms, routing table Longest Prefix Match (LPM), and Radix tree/Trie data structures.
- `visual-explanation.md`: Bitfield division of Network ID vs Host ID and subnet tree partitioning.
- `examples/`:
  - `cidr_subnet_calculator.cpp`: Production C++ CIDR subnet analyzer and IP range membership evaluator.
- `exercises/`: VLSM design problems and IP allocation challenges.
- `solutions/`: Complete step-by-step mathematical subnetting solutions.
- `interview-questions.md`: FAANG interview questions (e.g., VPC design, Longest Prefix Match, RFC 1918 NAT exhaustion).
- `common-mistakes.md`: Off-by-one errors in `/31` and `/32`, endianness bugs in bitwise IP manipulation, subnet overlap collisions.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 01: OSI & TCP/IP Model](../01-osi-and-tcp-ip-model/README.md) | [Computer Networks Hub](../README.md) | **[Module 03: Ethernet, ARP & Switching](../03-ethernet-arp-and-switching/README.md)** |
<!-- /Chapter Navigation Footer -->

