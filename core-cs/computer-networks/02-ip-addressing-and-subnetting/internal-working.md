# Internal Working: Subnet Bitwise Math & Longest Prefix Match

---

## 1. Subnet Bitwise Mathematical Operations

Every subnet calculation in the OS kernel or network switch boils down to fast 32-bit bitwise ALU instructions:

### 1. Generating the Subnet Mask from Prefix Length $P$:
```cpp
uint32_t mask = (P == 0) ? 0 : (~0U << (32 - P));
```
For example, with $P = 26$:
- `32 - 26 = 6`
- Shift `0xFFFFFFFF` left by 6: `11111111.11111111.11111111.11000000` $\implies$ `255.255.255.192` (`0xFFFFFFC0`).

### 2. Computing the Network Address:
$$\text{Network} = \text{IP} \ \& \ \text{Mask}$$
All host bits are masked to zero. This identifies the wire subnet and cannot be assigned to an individual host interface.

### 3. Computing the Broadcast Address:
$$\text{Broadcast} = \text{Network} \mid (\sim \text{Mask})$$
All host bits are set to one. Packets sent to this address are received by every host on the subnet.

### 4. Usable Host Range:
- **First Usable Host**: $\text{Network} + 1$
- **Last Usable Host**: $\text{Broadcast} - 1$
- **Total Capacity**: $2^{32 - P}$
- **Usable Capacity**: $2^{32 - P} - 2$ (Except for `/31` point-to-point links per RFC 3021, where both addresses are usable).

### 5. Checking Subnet Membership:
A target IP $T$ belongs to subnet $(S, \text{Mask})$ if and only if:
$$(T \ \& \ \text{Mask}) == (S \ \& \ \text{Mask})$$

---

## 2. Longest Prefix Match (LPM) in Router Forwarding Tables

When a router receives a packet with destination IP `192.168.1.130`, its forwarding table may match multiple overlapping routes:

| Route Entry | CIDR | Subnet Mask | Interface |
| :--- | :--- | :--- | :--- |
| Route A | `0.0.0.0/0` (Default Route) | `0.0.0.0` | `eth0` (Internet Gateway) |
| Route B | `192.168.0.0/16` | `255.255.0.0` | `eth1` (Corporate Campus) |
| Route C | `192.168.1.128/25` | `255.255.255.128` | `eth2` (Engineering Server Farm) |

### The Longest Prefix Match Rule:
The router selects the route with the **most specific (longest) prefix length**.
- Route A matches (prefix length = 0).
- Route B matches (prefix length = 16).
- Route C matches:
  - `192.168.1.130 & 255.255.255.128` = `192.168.1.128`. Matches! (prefix length = 25).
- **Winner**: **Route C (`/25`)** because $25 > 16 > 0$. The packet is routed out through `eth2`.

### Hardware Implementation:
- Software routers (Linux kernel) use **Radix Trees / LC-Tries** for $O(K)$ lookup.
- Hardware core routers use **TCAM (Ternary Content Addressable Memory)**, evaluating all 1,000,000+ routes in parallel in a single clock cycle (~2 nanoseconds).
