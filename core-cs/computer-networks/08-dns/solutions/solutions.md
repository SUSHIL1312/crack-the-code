# Solutions: Domain Name System (DNS) Exercises

---

### Solution 1: Zero-Downtime DNS Migration Timeline

1. **Step 1: T - 72 Hours (Wednesday 02:00 UTC) — Shrink TTL**:
   - Change `store.example.com IN A 203.0.113.50` TTL from `86400` (24h) down to `300` (5 minutes).
   - Why 72 hours before? Any resolver that cached the old record on Tuesday will have its 24-hour cache expire by Thursday. By Friday, all recursive resolvers globally will have refreshed their cache with the new 300s TTL.

2. **Step 2: T - 0 Hours (Saturday 02:00 UTC) — Cutover**:
   - Change the record to `store.example.com IN CNAME d1234.cloudfront.net.` with $\text{TTL} = 300$.
   - Because all resolvers globally cached a 300-second TTL, within **exactly 5 minutes**, $100\%$ of global recursive resolvers expire their old on-premise IP and fetch the new CloudFront CNAME!

3. **Step 3: Verification & Rollback Window (Saturday 02:05 - 04:00 UTC)**:
   - Monitor error rates and CloudFront logs.
   - If a catastrophic bug is found, roll back the CNAME to `203.0.113.50` with $\text{TTL} = 300$. Global traffic reverts within 5 minutes.

4. **Step 4: T + 48 Hours (Monday 02:00 UTC) — Restore Standard Production TTL**:
   - Once the migration is confirmed stable, increase TTL back to `86400` (24 hours) or `3600` (1 hour) to reduce DNS lookup latency and cut Route 53 / Cloudflare DNS query billing costs.

---

### Solution 2: Kaminsky DNS Cache Poisoning

1. **Why Traditional Spoofing Failed**:
   - If `bank.com` is already cached in the resolver with $\text{TTL} = 86400$, the resolver will not issue any outgoing queries for `bank.com` for 24 hours. The attacker only has a tiny fraction-of-a-second window once per day to guess the 16-bit Transaction ID.

2. **Kaminsky's Subdomain Bypass**:
   - Kaminsky queries random subdomains: `random1.bank.com`, `random2.bank.com`, etc.
   - Because these subdomains are guaranteed **not cached**, the resolver is forced to immediately issue an iterative UDP query to `bank.com`'s authoritative server for each request.
   - The attacker can flood the resolver with thousands of queries and fake replies per second!

3. **Poisoning the Entire Zone via Additional Records**:
   - In the fake forged reply for `random1.bank.com`, the attacker includes an **Authority Section**:
     `bank.com. IN NS ns.attacker.com.`
     and an **Additional Section**:
     `ns.attacker.com. IN A 6.6.6.6`
   - If the 16-bit Transaction ID matches on just **one** of the thousands of random attempts, the resolver accepts the reply and **poisons the nameserver pointer for the ENTIRE `bank.com` zone**! All subsequent queries for `www.bank.com`, `mail.bank.com`, and `login.bank.com` now go to Attacker's nameserver.

4. **Universal Defenses Deployed**:
   1. **Source Port Randomization (RFC 5452)**: Resolvers previously sent queries from fixed UDP port 53. Modern resolvers randomize the 16-bit UDP source port across the full range ($1024 - 65535$). The attacker now has to guess both the 16-bit Transaction ID and the 16-bit Source Port ($2^{32} \approx 4.3 \text{ billion combinations}$), making brute-force spoofing statistically impossible over bandwidth constraints.
   2. **DNSSEC**: Cryptographic signatures (`RRSIG`) prevent any forged response from being accepted, regardless of matching IDs or ports.
