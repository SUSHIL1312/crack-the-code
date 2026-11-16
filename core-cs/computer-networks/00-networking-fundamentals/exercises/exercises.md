# Hands-On Exercises: Networking Fundamentals

---

### Exercise 1: Geostationary Satellite vs Low Earth Orbit (LEO) Delay
**Problem**:
A geostationary satellite (GEO) orbits at an altitude of $35,786 \text{ km}$ above the equator. A modern LEO satellite (e.g. Starlink) orbits at an altitude of $550 \text{ km}$.
1. Calculate the minimum theoretical one-way propagation delay and Round-Trip Time (RTT) for a signal traveling from a ground station to the satellite and back to a ground station in vacuum ($s = 3 \times 10^8 \text{ m/s}$) for both GEO and LEO.
2. Explain why GEO satellites cannot support interactive real-time gaming or high-frequency trading.

---

### Exercise 2: End-to-End Packet Delay Across Multiple Store-and-Forward Routers
**Problem**:
A host sends a $10 \text{ MB}$ file over a path composed of 3 links separated by 2 store-and-forward routers.
- Each link has a transmission rate $R = 10 \text{ Mbps}$.
- Each link has a length of $1,000 \text{ km}$ ($s = 2 \times 10^8 \text{ m/s}$).
- Each router has a processing delay of $1 \text{ ms}$.
- Queuing delay is zero.
- Compare the total time required to transfer the file under two strategies:
  - **Strategy A**: Transfer as one single huge packet of 10 MB.
  - **Strategy B**: Segment the file into $10,000$ packets of $1 \text{ KB}$ each (pipelined packet switching).

---

### Exercise 3: Bandwidth-Delay Product (BDP) and Buffer Sizing
**Problem**:
You are designing a cross-country data replication pipeline between a primary database in Northern Virginia (`us-east-1`) and a standby replica in Oregon (`us-west-2`).
- Link Bandwidth: $40 \text{ Gbps}$.
- Measured RTT: $70 \text{ ms}$.
1. Compute the Bandwidth-Delay Product (BDP) in megabytes.
2. If the operating system's default TCP receive window is capped at $64 \text{ KB}$ (without TCP window scaling), calculate the maximum achievable throughput.
3. What percentage of the 40 Gbps link is wasted due to the buffer limitation?
