# Module 08: The Domain Name System (DNS)

<!-- Chapter Navigation Header -->
| ⬅️ [Module 07: Sockets Programming](../07-sockets-programming/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Module 09: HTTP & HTTPS](../09-http-and-https/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## Overview
The Domain Name System (DNS) is a globally distributed, hierarchical, fault-tolerant database that maps human-readable domain names (e.g., `api.google.com`) to machine-routable IP addresses (e.g., `142.250.190.46` or `2607:f8b0:4005:808::200e`). It is the very first step in virtually every internet interaction.

In this module, you will master the domain hierarchy, iterative vs recursive resolution algorithms, DNS record types (`A`, `AAAA`, `CNAME`, `MX`, `TXT`, `SRV`), Time-To-Live (TTL) caching dynamics, DNSSEC authenticity, and DNS over HTTPS/TLS (DoH/DoT).

---

## Learning Objectives
1. **The Global DNS Hierarchy**: Dissect Root Name Servers (the 13 root letters using BGP Anycast), Top-Level Domains (TLDs), and Authoritative Name Servers.
2. **Resolution Mechanics**: Contrast Recursive Resolvers (e.g. `8.8.8.8`, `1.1.1.1`) with Iterative Referral Queries.
3. **Core DNS Record Catalog**:
   - `A` (IPv4) & `AAAA` (IPv6)
   - `CNAME` (Canonical Name Aliases) & the Zone Apex limitation
   - `MX` (Mail Exchange & Priorities)
   - `TXT` (SPF, DKIM, Domain Verification)
   - `SRV` (Microservice Discovery)
4. **Caching & TTL Optimization**: Master multi-tier caching (Browser cache $\to$ OS cache $\to$ Resolver cache $\to$ Authoritative cache) and Negative Caching (RFC 2308).
5. **DNS Security & Privacy**: Analyze DNS Cache Poisoning (Kaminsky attack), DNSSEC cryptographic chains of trust, and encrypted DNS (DoH / DoT).

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of DNS architecture, resolution modes, record specifications, and DNSSEC.
- `internal-working.md`: Wire packet binary format (RFC 1035), label compression pointers (`0xC0`), and BGP Anycast routing for root servers.
- `visual-explanation.md`: Visual walk of iterative DNS resolution and the DNSSEC chain of trust.
- `examples/`:
  - `dns_resolver_and_cache.cpp`: Production C++ in-memory DNS resolver and LRU cache with TTL expiration and negative caching.
- `exercises/`: DNS packet decoding and zone configuration exercises.
- `solutions/`: Reference solutions.
- `interview-questions.md`: FAANG interview questions (e.g. "What happens when you type a URL into a browser?", "Why can't you put a CNAME on the root apex domain?").
- `common-mistakes.md`: Long TTL traps during migrations, CNAME apex conflicts, DNS resolution blocking event loops.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 07: Sockets Programming](../07-sockets-programming/README.md) | [Computer Networks Hub](../README.md) | **[Module 09: HTTP & HTTPS](../09-http-and-https/README.md)** |
<!-- /Chapter Navigation Footer -->

