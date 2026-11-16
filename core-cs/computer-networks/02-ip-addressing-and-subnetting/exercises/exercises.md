# Hands-On Exercises: IP Addressing & Subnetting

---

### Exercise 1: Multi-Tier Cloud VPC Subnetting (VLSM Design)
**Goal**: Design a secure, production-grade cloud Virtual Private Cloud (VPC) network in AWS/GCP.

**Requirements**:
You are assigned the root CIDR block `10.100.0.0/16`. You must partition this block across 3 Availability Zones (`AZ-a`, `AZ-b`, `AZ-c`) to support:
1. **Public Subnets**: 3 subnets (one per AZ) for internet-facing ALB / NAT Gateways, each requiring at least 100 usable IP addresses.
2. **Application Subnets (Private)**: 3 subnets for microservices/Kubernetes worker nodes, each requiring at least 1,000 usable IP addresses.
3. **Database Subnets (Isolated Private)**: 3 subnets for RDS / Aurora clusters, each requiring at least 25 usable IP addresses.
4. Calculate the optimal prefix length for each tier, assign non-overlapping CIDR blocks, and compute the remaining unused address capacity for future expansion.

---

### Exercise 2: Point-to-Point Links in High-Density Datacenters (RFC 3021)
**Goal**: Explain and calculate router-to-router point-to-point links.

**Requirements**:
1. Traditionally, point-to-point router links used `/30` subnets (`255.255.255.252`). How many total IP addresses exist in a `/30`? How many are usable hosts? What percentage of addresses are wasted as Network and Broadcast identifiers?
2. Explain RFC 3021 (`/31` subnets). How does it eliminate waste? What are the network and broadcast addresses in a `/31` link?
3. If a hyperscale datacenter has 20,000 point-to-point router interconnects, how many IP addresses are saved by upgrading from `/30` to `/31`?

---

### Exercise 3: IPv6 Address Shortening and Expansion Engine
**Goal**: Implement an algorithm in C++ or Python that accepts an arbitrary uncompressed or compressed IPv6 address and outputs:
1. The fully expanded canonical 32-hex-digit representation (with leading zeros and colons).
2. The shortest valid compressed representation using RFC 5952 canonical formatting rules.
