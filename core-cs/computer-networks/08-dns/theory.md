# Theoretical Foundations: The Domain Name System (DNS)

---

## 1. The Global DNS Hierarchy

The domain namespace is an inverted tree structure with the Root (`.`) at the top:

```
                            . (Root Zone)
                           /      |      \
                         .com    .org    .uk (TLDs)
                         /        |        \
                   google.com  eff.org   co.uk (Second-Level Domains)
                   /       \
             mail.google  api.google (Subdomains)
```

1. **Root Name Servers (`.`)**:
   - 13 logical IP authorities named `a.root-servers.net` through `m.root-servers.net`.
   - Though there are only 13 IP addresses, they are operated across **over 1,500 physical server instances worldwide** using **BGP Anycast routing**!
   - Responsible for directing queries to the appropriate TLD nameservers.
2. **Top-Level Domain (TLD) Servers**:
   - Generic TLDs (gTLDs: `.com`, `.net`, `.org`, `.app`).
   - Country Code TLDs (ccTLDs: `.uk`, `.de`, `.in`, `.cn`).
   - Managed by designated registries (e.g. Verisign manages `.com`).
3. **Authoritative Name Servers**:
   - The definitive source for a specific organization's domain records (e.g. AWS Route 53, Cloudflare, Google Cloud DNS).
   - If an authoritative nameserver says an `A` record does not exist (`NXDOMAIN`), that answer is definitive.

---

## 2. Recursive vs Iterative Resolution

When a user visits `https://news.google.com`:

```
[ Client Browser ]
       |
       | 1. Query: "news.google.com" (Recursive)
       v
[ Recursive Resolver (e.g. 8.8.8.8) ]
       |
       | 2. Iterative: "Where is news.google.com?"
       +----------------------------------------> [ Root Nameserver (.) ]
       |<----------------------------------------+
       | Response: Referral to .com TLD (IP of a.gtld-servers.net)
       |
       | 3. Iterative: "Where is news.google.com?"
       +----------------------------------------> [ .com TLD Nameserver ]
       |<----------------------------------------+
       | Response: Referral to google.com Auth NS (IP of ns1.google.com)
       |
       | 4. Iterative: "Where is news.google.com?"
       +----------------------------------------> [ Authoritative Nameserver (ns1.google.com) ]
       |<----------------------------------------+
       | Response: 142.250.190.46 (TTL = 300)
       |
       | 5. Final Answer: 142.250.190.46
       v
[ Client Browser ]
```

- **Recursive Query**: *"Please do whatever it takes to find the final IP answer and bring it back to me."* (Issued by Client to Resolver).
- **Iterative Query**: *"If you don't know the answer, tell me the next nameserver closer to the answer that I should ask."* (Issued by Resolver to Root, TLD, and Authoritative servers).

---

## 3. Essential DNS Resource Records (RR)

| Record Type | RFC | Purpose | Example |
| :--- | :--- | :--- | :--- |
| **`A`** | RFC 1035 | Maps hostname to 32-bit IPv4 address. | `api.example.com. IN A 93.184.216.34` |
| **`AAAA`** | RFC 3596 | Maps hostname to 128-bit IPv6 address. | `api.example.com. IN AAAA 2606:2800:220:1:248:1893:25c8:1946` |
| **`CNAME`** | RFC 1035 | Canonical Name: Alias to another domain. | `www.example.com. IN CNAME example.com.` |
| **`MX`** | RFC 1035 | Mail Exchange: Specifies mail servers with priority. | `example.com. IN MX 10 mail.example.com.` |
| **`NS`** | RFC 1035 | Name Server: Delegates authority to nameservers. | `example.com. IN NS ns1.awsdns.com.` |
| **`TXT`** | RFC 1035 | Freeform text: SPF (`v=spf1`), DKIM, DMARC, domain verification. | `example.com. IN TXT "v=spf1 include:_spf.google.com ~all"` |
| **`PTR`** | RFC 1035 | Pointer: Reverse DNS lookup (IP to hostname). | `34.216.184.93.in-addr.arpa. IN PTR example.com.` |
| **`SOA`** | RFC 1035 | Start of Authority: Primary master, serial number, TTL timers. | `example.com. IN SOA ns1.example.com. admin.example.com. (2024010101 7200 3600 1209600 3600)` |
| **`SRV`** | RFC 2782 | Service Discovery: Host, port, priority, weight. | `_sip._tcp.example.com. IN SRV 10 60 5060 bigbox.example.com.` |

---

## 4. The CNAME Apex Problem

Why can't you put a `CNAME` record on the root apex domain (e.g. `example.com`), only on subdomains (`www.example.com`)?
- **RFC 1034 Rule**: If a domain has a `CNAME` record, it **cannot coexist with any other record type for that exact name**.
- **The Conflict**: A root zone (`example.com`) **must** contain an `SOA` record and `NS` records to function as a valid DNS zone.
- If you place a `CNAME` at `example.com`, it invalidates the `SOA` and `NS` records, breaking DNS for the entire domain!
- **Modern Cloud Solution**: Cloud providers invented **ALIAS / ANAME** records (e.g. Route 53 Alias, Cloudflare CNAME Flattening). In ALIAS records, the nameserver dynamically resolves the target domain behind the scenes and returns synthetic `A` records to the client, preserving compliance with RFC 1034!
