# Hands-On Exercises: Network Performance & Optimization

---

### Exercise 1: Linux Memory Budgeting for 100,000 Concurrent TCP Sockets
**Goal**: Calculate host RAM requirements for large-scale C100K servers without triggering the Linux OOM killer.

**Scenario**:
You are architecting an edge reverse proxy (e.g. Envoy / Nginx) expected to maintain 100,000 idle-to-active concurrent client connections.
- Default `tcp_rmem` = `4096 87380 6291456` (min, default, max).
- Default `tcp_wmem` = `4096 16384 4194304`.
- Kernel overhead per socket `struct sock`: approximately 3.2 KB.
1. If all 100,000 sockets operate at the default memory allocation ($87 \text{ KB} + 16 \text{ KB} + 3.2 \text{ KB}$), how many gigabytes of host RAM are consumed?
2. What happens if all 100,000 connections simultaneously experience burst downloads and expand to their maximum `tcp_rmem` (6 MB) and `tcp_wmem` (4 MB)?
3. What sysctl parameters constrain total global TCP memory (`tcp_mem`) to protect the operating system from OOM collapse?

---

### Exercise 2: CDN Point of Presence (PoP) Capacity Planning
**Goal**: Model the bandwidth and origin offload of a global video-on-demand service.

**Scenario**:
A media streaming company serves 50 Terabytes of video daily.
- 85% of traffic is concentrated in 20 viral video titles.
- 15% represents long-tail niche videos.
- Cache Hit Ratio at CDN Edge = 92%.
1. How many Terabytes of outbound traffic are delivered directly from CDN edge caches vs forwarded to the origin database servers?
2. If the origin server egress bandwidth costs \$0.08 per GB while CDN bandwidth costs \$0.015 per GB, calculate the daily cost savings delivered by the CDN.
3. If an emergency cache purge is executed, what is the "Thundering Herd" / Cache Stampede risk to the origin servers? How is it prevented?
