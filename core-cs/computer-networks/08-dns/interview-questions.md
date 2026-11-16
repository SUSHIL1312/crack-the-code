# Domain Name System (DNS): Top FAANG Interview Questions

### Q1: Exactly what happens from the moment a user types `https://www.google.com` into their browser URL bar up until the DNS resolution finishes?
**Answer**:
1. **HSTS Preload Check**: Browser checks its internal HTTP Strict Transport Security list to enforce HTTPS before making any requests.
2. **Browser DNS Cache Lookup**:
   - Chrome checks its internal in-memory DNS cache (`chrome://net-internals/#dns`). If cached and unexpired, resolution finishes in $0 \text{ ms}$.
3. **OS System Cache Lookup**:
   - The browser invokes POSIX `getaddrinfo("www.google.com")`.
   - The OS checks `/etc/hosts` for local overrides.
   - The OS checks the local caching daemon (`systemd-resolved` on Linux, `mDNSResponder` on macOS).
4. **Local Router / Gateway Query**:
   - If not cached, the OS sends a UDP packet to the configured DNS resolver in `/etc/resolv.conf` (often the local Wi-Fi router `192.168.1.1` running `dnsmasq`).
5. **Recursive Resolver Processing**:
   - The query reaches the upstream recursive resolver (ISP resolver or public resolver `8.8.8.8` / `1.1.1.1`).
   - Resolver checks its shared cache. If unexpired, returns answer immediately.
6. **Iterative Resolution Walk (if cold)**:
   - **Root Nameserver**: Resolver queries one of the 13 Root IP Anycast servers (`a.root-servers.net`). Root returns referral to `.com` TLD servers (`a.gtld-servers.net`).
   - **TLD Nameserver**: Resolver queries `.com` TLD server. TLD returns referral to Google's authoritative nameservers (`ns1.google.com`, `ns2.google.com`).
   - **Authoritative Nameserver**: Resolver queries `ns1.google.com` for `A` and `AAAA` records for `www.google.com`.
7. **Return & Cache**:
   - Google Authoritative server responds with `A 142.250.190.46` (with $\text{TTL} = 300$).
   - Recursive resolver caches the answer for 300 seconds and returns it to the client OS.
   - The OS caches the answer and returns it to the browser.
   - Browser initiates the TCP 3-way handshake to `142.250.190.46:443`.

---

### Q2: What is the CNAME Apex / Zone Apex limitation, and how do modern cloud providers work around it?
**Answer**:
- **The Problem**:
  - The apex domain (also known as the naked or root domain, e.g. `example.com` without `www`) must contain an `SOA` (Start of Authority) and `NS` (Name Server) record.
  - RFC 1034 section 3.6.2 explicitly states: *"If a CNAME RR is present at a node, no other data should be present; this ensures that the data for a canonical name and its aliases cannot be different."*
  - Putting a `CNAME` at the apex would make `SOA` and `NS` illegal, breaking the zone completely.
- **Why It Matters Today**: Modern cloud services (AWS CloudFront, Heroku, Shopify, Vercel) provide dynamically scaling hostnames (e.g. `d1234.cloudfront.net`) rather than static IP addresses. You cannot point an `A` record to an IP that changes every hour.
- **The Workaround (ALIAS / ANAME Records)**:
  - AWS Route 53 introduced **Route 53 Alias Records**. Cloudflare introduced **CNAME Flattening**.
  - To the user, it behaves like a CNAME pointing `example.com -> d1234.cloudfront.net`.
  - Internally at the nameserver: When a client queries `example.com A`, the authoritative nameserver resolves `d1234.cloudfront.net` in real-time, extracts the current IP addresses, and returns synthetic **`A` records** to the client. The client sees valid `A` records and RFC 1034 is preserved!

---

### Q3: What is DNS over HTTPS (DoH) and DNS over TLS (DoT), and what privacy vulnerability do they solve?
**Answer**:
- **The Vulnerability of Traditional DNS**:
  - Traditional DNS operates over cleartext unencrypted UDP on Port 53.
  - Anyone on the local network (Wi-Fi eavesdroppers), ISPs, and government intelligence agencies can passively log every domain name visited by an IP, building a detailed profile of the user's browsing activity. Furthermore, malicious ISPs can hijack port 53 traffic to inject censorship redirects or advertisements.
- **The Encrypted DNS Solution**:
  - **DoT (RFC 7858 - Port 853)**: Wraps standard DNS wire packets inside a dedicated TLS connection on port 853.
  - **DoH (RFC 8484 - Port 443)**: Transmits DNS queries as HTTP/2 or HTTP/3 `POST`/`GET` requests over HTTPS on standard port 443.
  - **Advantage of DoH**: Because it blends into regular HTTPS web traffic on port 443, ISPs and corporate firewalls cannot easily inspect, throttle, or block DoH without blocking all web traffic.
