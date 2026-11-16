# Hands-On Exercises: Domain Name System (DNS)

---

### Exercise 1: Zero-Downtime DNS Migration Plan
**Goal**: Design a production DNS record migration strategy for moving a high-traffic e-commerce platform (`store.example.com`) from on-premise datacenter IP `203.0.113.50` to AWS CloudFront CDN (`d1234.cloudfront.net`).

**Parameters**:
- Current `store.example.com` has an `A` record with $\text{TTL} = 86,400 \text{ seconds}$ (24 hours).
- The migration maintenance window is scheduled for Saturday at 02:00 AM UTC.
- Write a step-by-step timeline of TTL modifications starting 3 days before migration through 2 days after migration to guarantee that 99.9% of global user traffic shifts over within 5 minutes of switchover, with immediate rollback capability.

---

### Exercise 2: Kaminsky DNS Cache Poisoning Analysis
**Goal**: Analyze the classic 2008 Dan Kaminsky DNS vulnerability and modern defenses.

**Scenario**:
An attacker wants to poison a recursive resolver (`8.8.8.8`) so that queries for `bank.com` return the attacker's IP (`6.6.6.6`).
1. Why were traditional DNS spoofing attacks difficult if the attacker had to wait for the target domain's legitimate TTL to expire?
2. How did Kaminsky circumvent TTL caching by querying random non-existent subdomains (`1234.bank.com`, `5678.bank.com`)?
3. How did Kaminsky forge the Authoritative NS section to poison the entire zone?
4. What two critical mitigations were universally deployed across all DNS resolvers worldwide to defeat this attack?
