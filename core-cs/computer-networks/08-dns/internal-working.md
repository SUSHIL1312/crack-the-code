# Internal Working: DNS Wire Format, Compression, & BGP Anycast

---

## 1. DNS Wire Format (RFC 1035)

A DNS message (transmitted over UDP port 53 or TCP port 53) has a fixed 12-byte header followed by 4 variable-length sections:

```
+---------------------------------------------------------------+
|                      Header (12 Bytes)                        |
|  - ID (16 bits): Matches request with reply                   |
|  - Flags (16 bits): QR (0=query, 1=resp), Opcode, AA, TC, RD, |
|                     RA, RCODE (0=NoErr, 3=NXDomain)           |
|  - QDCOUNT (16 bits): Number of entries in Question section   |
|  - ANCOUNT (16 bits): Number of resource records in Answer    |
|  - NSCOUNT (16 bits): Authority records count                 |
|  - ARCOUNT (16 bits): Additional records count                |
+---------------------------------------------------------------+
|                      Question Section                         |
|  - QNAME: Length-prefixed domain labels                       |
|  - QTYPE: 1 (A), 28 (AAAA), 5 (CNAME), 15 (MX), etc.          |
|  - QCLASS: 1 (IN - Internet)                                  |
+---------------------------------------------------------------+
|                      Answer Section (RRs)                     |
+---------------------------------------------------------------+
|                      Authority Section (RRs)                  |
+---------------------------------------------------------------+
|                      Additional Section (RRs)                 |
+---------------------------------------------------------------+
```

---

## 2. DNS Label Encoding & Compression Pointers

Domain names in DNS wire packets are encoded as a series of **length-prefixed labels** terminated by a null byte (`0x00`).
For example, `api.google.com` is encoded on the wire as:
```
\x03 a p i \x06 g o o g l e \x03 c o m \x00
```

### Compression Pointers (`0xC0`):
To prevent redundant repetition of long domain suffixes in the Answer, Authority, and Additional sections of a packet, RFC 1035 uses **Message Compression Pointers**:
- If the two highest bits of a length byte are `11` (i.e. `0xC0`), the remaining 14 bits represent a **byte offset from the start of the DNS header**.
- `0xC00C`: Jump to byte offset 12 in the packet and read the domain name from there! This reduces DNS response packet sizes by over 60%.

---

## 3. How 13 Root Server IPs Scale Globally via BGP Anycast

Historically, DNS over UDP was restricted to a maximum packet size of **512 bytes** (without EDNS0).
A DNS response listing all root server names and IP addresses could fit at most 13 server addresses within 512 bytes. Hence, only 13 IP addresses (`198.41.0.4` down to `202.12.27.33`) were assigned.

### BGP Anycast:
- **Anycast** allows **multiple distinct physical servers around the world to announce the EXACT same IP address** to BGP internet routers.
- When an ISP router in Tokyo sends a packet to `198.41.0.4` (`a.root-servers.net`), BGP routes it to the local Tokyo instance (1 ms away).
- When a client in Frankfurt queries `198.41.0.4`, BGP routes it to the Frankfurt instance.
- Today, the 13 root IP addresses are backed by **over 1,700 high-capacity server clusters** distributed across six continents, providing near-infinite DDoS resilience and low-latency access worldwide.
