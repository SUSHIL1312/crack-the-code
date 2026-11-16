# Visual Explanation: DNS Resolution Chain & Caching Tiers

---

## 1. Multi-Tiered DNS Caching Hierarchy

When an application resolves a domain, it queries progressively broader caches before hitting authoritatives:

```
+-------------------------------------------------------------+
| Tier 1: Browser Cache (Chrome / Firefox in-memory cache)    |
|   TTL: 60 seconds (chrome://net-internals/#dns)             |
+-------------------------------------------------------------+
        |  Cache Miss
        v
+-------------------------------------------------------------+
| Tier 2: Operating System Cache (systemd-resolved / mDNS)    |
|   TTL: Honored from DNS TTL response                        |
+-------------------------------------------------------------+
        |  Cache Miss
        v
+-------------------------------------------------------------+
| Tier 3: Local Router / Gateway Cache (Home / Office Wi-Fi)  |
|   e.g. dnsmasq on 192.168.1.1                               |
+-------------------------------------------------------------+
        |  Cache Miss
        v
+-------------------------------------------------------------+
| Tier 4: Recursive Resolver Cache (ISP / 8.8.8.8 / 1.1.1.1)  |
|   Shared across millions of regional users                  |
+-------------------------------------------------------------+
        |  Cache Miss
        v
+-------------------------------------------------------------+
| Tier 5: Authoritative Nameserver (AWS Route 53 / Cloudflare)|
|   Definitive truth directly from the domain zone file       |
+-------------------------------------------------------------+
```
Over 95% of real-world internet DNS queries terminate at Tier 1 or Tier 4, shielding root and authoritative nameservers from trillions of queries daily.

---

## 2. DNSSEC Cryptographic Chain of Trust

```
[ Root Zone (.) ]
  - Root Key Signing Key (KSK) signed at public ceremonial key ceremony
  - Signs Zone Signing Key (ZSK)
  - Signs DS Record (Delegation Signer) for .com
        |
        v
[ .com TLD Zone ]
  - .com DS record proves authenticity of .com KSK
  - Signs DS Record for example.com
        |
        v
[ example.com Authoritative Zone ]
  - example.com DS record proves authenticity of example.com KSK
  - Signs RRSIG (Resource Record Signature) for "api.example.com A 93.184.216.34"
        |
        v
[ Recursive Resolver Verifies RRSIG ]
  - Validates digital signatures all the way back up to the Root Trust Anchor.
  - Guarantees 0% chance of DNS Cache Poisoning or MitM spoofing!
```
