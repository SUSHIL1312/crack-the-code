# Solutions: IP Addressing & Subnetting Exercises

---

### Solution 1: Cloud VPC VLSM Partitioning

Root VPC: `10.100.0.0/16` ($65,536$ total IP addresses).

#### Tier Sizing:
1. **Application Subnets** (Requires $\ge 1,000$ hosts per AZ):
   - $2^{10} - 2 = 1,022$ usable hosts ($32 - 10 = \mathbf{/22}$, size 1,024).
   - `App-AZ-a`: `10.100.0.0/22` (Range: `10.100.0.1` - `10.100.3.254`)
   - `App-AZ-b`: `10.100.4.0/22` (Range: `10.100.4.1` - `10.100.7.254`)
   - `App-AZ-c`: `10.100.8.0/22` (Range: `10.100.8.1` - `10.100.11.254`)

2. **Public Subnets** (Requires $\ge 100$ hosts per AZ):
   - $2^7 - 2 = 126$ usable hosts ($32 - 7 = \mathbf{/25}$, size 128).
   - `Public-AZ-a`: `10.100.16.0/25` (Range: `10.100.16.1` - `10.100.16.126`)
   - `Public-AZ-b`: `10.100.16.128/25` (Range: `10.100.16.129` - `10.100.16.254`)
   - `Public-AZ-c`: `10.100.17.0/25` (Range: `10.100.17.1` - `10.100.17.126`)

3. **Database Subnets** (Requires $\ge 25$ hosts per AZ):
   - $2^5 - 2 = 30$ usable hosts ($32 - 5 = \mathbf{/27}$, size 32).
   - `DB-AZ-a`: `10.100.20.0/27` (Range: `10.100.20.1` - `10.100.20.30`)
   - `DB-AZ-b`: `10.100.20.32/27` (Range: `10.100.20.33` - `10.100.20.62`)
   - `DB-AZ-c`: `10.100.20.64/27` (Range: `10.100.20.65` - `10.100.20.94`)

4. **Remaining Space**:
   - Total addresses assigned: $(3 \times 1024) + (3 \times 128) + (3 \times 32) = 3072 + 384 + 96 = 3,552$ addresses.
   - Unused space: $65,536 - 3,552 = \mathbf{61,984 \text{ addresses}}$ ($94.58\%$ capacity preserved for future expansion!).

---

### Solution 2: RFC 3021 Point-to-Point Links

1. **Traditional `/30` Subnet**:
   - Total addresses: $2^{32 - 30} = 4$ addresses.
   - Usable hosts: $4 - 2 = 2$ hosts.
   - Wasted: 2 addresses ($50\%$ waste!) consumed by Network Address and Directed Broadcast.
2. **RFC 3021 `/31` Subnet**:
   - Total addresses: $2^{32 - 31} = 2$ addresses.
   - Usable hosts: **2 hosts**.
   - Network address: The lower address (e.g. `10.0.0.4`).
   - Broadcast: Not needed on point-to-point links (broadcast packets are sent to limited broadcast `255.255.255.255`).
   - Waste: **0% waste**!
3. **Datacenter Savings**:
   - For 20,000 links:
     - With `/30`: $20,000 \times 4 = 80,000 \text{ IP addresses}$.
     - With `/31`: $20,000 \times 2 = 40,000 \text{ IP addresses}$.
   - **Total IPs saved**: $\mathbf{40,000 \text{ IP addresses}}$ (equivalent to more than two entire `/18` blocks!).
