# Common Mistakes in IP Addressing & Subnetting

### 1. Forgetting Subnet Boundary Constraints (Arbitrary Starting IPs)
- **Mistake**: Attempting to create a `/24` subnet starting at an arbitrary IP like `192.168.1.50/24`.
- **Reality**: In binary arithmetic, a subnet network address **must have all host bits set to zero**.
  - For `/24`, the last octet must be `0` (`192.168.1.0/24`).
  - For `/26` (size 64), the last octet can only start at multiples of 64: `0`, `64`, `128`, or `192`. Writing `192.168.1.50/26` means the network address is still `192.168.1.0/26`.

---

### 2. Assuming `/32` Means No Usable Hosts
- **Mistake**: Thinking that because formula $2^{32 - P} - 2$ yields $2^0 - 2 = -1$, a `/32` subnet is invalid.
- **Reality**: A `/32` prefix represents a **single, specific host route** (e.g., `192.0.2.1/32` or loopback interface). It has exactly 1 usable address (the IP itself). It is widely used in BGP Anycast, loopback interfaces (`lo:0`), and container network interface host routes (Calico / Cilium).

---

### 3. Bit Shift Undefined Behavior on Prefix 0
- **Mistake**: Writing `uint32_t mask = ~0U << (32 - prefix);` without checking if `prefix == 0`.
- **Reality**: When `prefix == 0`, `32 - prefix = 32`. In C and C++, shifting a 32-bit integer by 32 bits is **Undefined Behavior (UB)** according to the ISO standard. On x86 processors, `x << 32` masks the shift count to 5 bits (`32 & 0x1F = 0`), resulting in `x << 0 = ~0U` instead of `0`!
- **Fix**: Use a ternary check: `(prefix == 0) ? 0 : (~0U << (32 - prefix))`.

---

### 4. Overlapping Subnets in Multi-Region VPC Peering
- **Mistake**: Creating VPC 1 in `us-east-1` with CIDR `10.0.0.0/16` and VPC 2 in `eu-west-1` with CIDR `10.0.0.0/16`, then attempting to peer them via AWS VPC Peering or VPN.
- **Reality**: Routers cannot route traffic when destination IP addresses are ambiguous. Overlapping CIDRs cannot be peered without complex Source/Destination NAT translation. Always partition enterprise address spaces globally from a single IPAM (IP Address Management) plan.
