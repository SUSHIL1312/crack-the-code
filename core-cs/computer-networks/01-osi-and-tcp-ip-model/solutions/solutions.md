# Solutions: OSI & TCP/IP Layer Models Exercises

---

### Solution 2: IP Fragmentation Calculations

- Payload: $4,000 \text{ bytes}$
- UDP Header: $8 \text{ bytes}$
- Total Layer 4 Data: $4,008 \text{ bytes}$
- IP MTU: $1,500 \text{ bytes}$
- Standard IPv4 Header: $20 \text{ bytes}$
- Maximum data payload per fragment: $1500 - 20 = 1480 \text{ bytes}$.
- Note: Fragment offset is measured in units of 8-byte blocks ($1480 / 8 = 185$, which is an integer divisible by 8).

#### Fragment Breakdown:
1. **Fragment 1**:
   - Contains: 20-byte IP header + 8-byte UDP header + 1472 bytes UDP payload = $1,480 \text{ bytes}$ IP payload.
   - Total Length: $1,500 \text{ bytes}$.
   - Flags: `MF = 1` (More Fragments).
   - Fragment Offset: $0$ ($0 / 8$).
2. **Fragment 2**:
   - Contains: 20-byte IP header + 1480 bytes UDP payload.
   - Total Length: $1,500 \text{ bytes}$.
   - Flags: `MF = 1` (More Fragments).
   - Fragment Offset: $185$ ($1480 / 8 = 185$).
   - *Does it contain a UDP header?* **No!** Transport layer headers (TCP/UDP) only exist in the first fragment (Offset 0). Intermediate routers and fragments #2 and #3 have no transport layer port information.
3. **Fragment 3**:
   - Remaining UDP payload: $4000 - (1472 + 1480) = 1,048 \text{ bytes}$.
   - Total Length: $20 + 1048 = 1,068 \text{ bytes}$.
   - Flags: `MF = 0` (Last Fragment).
   - Fragment Offset: $370$ ($(1480 + 1480) / 8 = 2960 / 8 = 370$).

---

### Solution 3: Protocol Overhead & Efficiency

1. **TCP Over Ethernet**:
   - Payload: 4 bytes
   - TCP Header: 20 bytes
   - IPv4 Header: 20 bytes
   - Ethernet Header + FCS: 14 + 4 = 18 bytes
   - Preamble + SFD + Interpacket Gap: 7 + 1 + 12 = 20 bytes (wire physical overhead)
   - Total On-Wire Frame Bytes: $4 + 20 + 20 + 18 = 62 \text{ bytes}$ (padded to 64 bytes minimum Ethernet frame size!).
   - Protocol Efficiency:
     $$\text{Efficiency}_{\text{TCP}} = \frac{4 \text{ bytes}}{64 \text{ bytes}} \times 100\% = \mathbf{6.25\%}$$
     (93.75% of the data sent is protocol packaging overhead!)

2. **UDP Over Ethernet**:
   - Payload: 4 bytes
   - UDP Header: 8 bytes
   - IPv4 Header: 20 bytes
   - Ethernet Header + FCS: 18 bytes
   - Subtotal: $4 + 8 + 20 + 18 = 50 \text{ bytes}$.
   - Because minimum Ethernet frame size is 64 bytes, Ethernet hardware pads 14 null bytes to reach 64 bytes.
   - Efficiency remains $\mathbf{6.25\%}$ due to minimum Ethernet frame padding rules ($64 \text{ bytes}$).
   - **Optimization Insight**: In high-performance games and microservices, small updates should be batched together to amortize header overhead.
