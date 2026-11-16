# Solutions: Network Performance & Optimization

---

### Solution 1: Linux Memory Budgeting for 100,000 Concurrent TCP Sockets

#### 1. Host RAM Consumption at Default Buffer Allocations
Each active socket requires memory for:
- Socket kernel structure (`struct sock` + metadata): $\approx 3.2\text{ KB}$
- Receive buffer default (`tcp_rmem[1]`): $87,380\text{ bytes} \approx 85.33\text{ KB}$
- Send buffer default (`tcp_wmem[1]`): $16,384\text{ bytes} \approx 16.00\text{ KB}$

Total memory per connection:
$$\text{RAM}_{\text{conn}} = 3.2\text{ KB} + 85.33\text{ KB} + 16.00\text{ KB} = 104.53\text{ KB}$$

For 100,000 concurrent sockets:
$$\text{RAM}_{\text{total}} = 100{,}000 \times 104.53\text{ KB} = 10{,}453{,}000\text{ KB} \approx 10.45\text{ GB}$$

Thus, approximately **10.45 GB** of host physical RAM is dedicated exclusively to TCP buffers and socket kernel structures under normal default operation.

#### 2. Worst-Case Buffer Expansion Risk
If all 100,000 connections experience simultaneous large-file streaming or packet drops causing window expansion to the maximum configured values:
- Maximum receive buffer (`tcp_rmem[2]`): $6{,}291{,}456\text{ bytes} = 6.0\text{ MB}$
- Maximum send buffer (`tcp_wmem[2]`): $4{,}194{,}304\text{ bytes} = 4.0\text{ MB}$
- Socket structure: $3.2\text{ KB} \approx 0.003\text{ MB}$

Total worst-case memory per connection:
$$\text{RAM}_{\text{conn, max}} \approx 6.0\text{ MB} + 4.0\text{ MB} = 10.0\text{ MB}$$

For 100,000 connections:
$$\text{RAM}_{\text{total, max}} = 100{,}000 \times 10.0\text{ MB} = 1{,}000{,}000\text{ MB} \approx 1{,}000\text{ GB} = 1\text{ TB!}$$

Without system-level guardrails, a machine with 64 GB or 128 GB of RAM will instantly exhaust kernel slab and page cache allocations, triggering the Linux Out-Of-Memory (`OOM`) killer and terminating essential services.

#### 3. Protecting the OS with `tcp_mem`
Linux protects itself using the global sysctl parameter `net.ipv4.tcp_mem = <min> <pressure> <max>`.
**Note**: Unlike `tcp_rmem` and `tcp_wmem` (which are specified in **bytes**), `tcp_mem` values are measured in **memory pages** (typically 4 KB on x86_64, or 16 KB on Apple Silicon).

- **min** (Low): Below this threshold of page allocations, the kernel applies no restrictions on TCP memory.
- **pressure**: When total TCP page allocations exceed this threshold, the kernel enters memory pressure mode. It stops auto-tuning socket windows upward and aggressively shrinks TCP receive and send windows back toward their minimum sizes.
- **max**: The absolute hard ceiling. If total TCP memory reaches `max`, new packet allocations are dropped, socket writes block or return `ENOBUFS`, and new connections fail to allocate buffers.

To size `tcp_mem` safely for a 64 GB server reserving 32 GB for networking:
$$\text{Target Max} = 32\text{ GB} = 34{,}359{,}738{,}368\text{ bytes} \div 4096\text{ bytes/page} = 8{,}388{,}608\text{ pages}$$
Configure:
```bash
# In /etc/sysctl.d/99-network.conf
net.ipv4.tcp_mem = 4194304 6291456 8388608
```

---

### Solution 2: CDN Point of Presence (PoP) Capacity Planning

#### 1. Daily Egress Traffic Breakdown
- Total Daily Video Delivered: $50\text{ TB} = 50{,}000\text{ GB}$
- Cache Hit Ratio at Edge: $92\%$ ($0.92$)
- Cache Miss Ratio to Origin: $1 - 0.92 = 8\%$ ($0.08$)

Traffic delivered directly from CDN Edge PoPs:
$$\text{Traffic}_{\text{CDN}} = 50\text{ TB} \times 0.92 = 46\text{ TB} = 46{,}000\text{ GB}$$

Traffic forwarded upstream to Origin servers:
$$\text{Traffic}_{\text{Origin}} = 50\text{ TB} \times 0.08 = 4\text{ TB} = 4{,}000\text{ GB}$$

#### 2. Daily Cost Savings Analysis
- **Without CDN (Direct Origin Serving)**:
  $$\text{Cost}_{\text{direct}} = 50{,}000\text{ GB} \times \$0.08/\text{GB} = \$4{,}000/\text{day}$$

- **With CDN**:
  - CDN Edge Egress Cost: $50{,}000\text{ GB} \times \$0.015/\text{GB} = \$750/\text{day}$
  - Origin Ingestion/Egress to CDN (4,000 GB miss): $4{,}000\text{ GB} \times \$0.08/\text{GB} = \$320/\text{day}$
  - Total Daily Cost With CDN: $\$750 + \$320 = \$1{,}070/\text{day}$

- **Daily Cost Savings**:
  $$\text{Savings} = \$4{,}000 - \$1{,}070 = \$2{,}930/\text{day}$$
  **Annual Savings**: $\$2{,}930 \times 365 = \$1{,}069{,}450/\text{year}$ (a 73.25% cost reduction).

#### 3. Thundering Herd Risk & Prevention
**The Problem**:
When a viral video (e.g. 50,000 concurrent requests/sec) expires or is purged from the CDN edge cache simultaneously across all edge servers, every edge PoP simultaneously sends a cache miss upstream to the origin database/storage tier. 50,000 requests hit the origin within milliseconds, overloading CPU, connection pools, and database read replicas—causing cascading outages.

**Prevention Strategies**:
1. **Request Collapsing (Edge Request Coalescing)**:
   - When 1,000 clients request the same uncached URI simultaneously, the edge node issues **only one single upstream HTTP request** to the origin.
   - The edge holds the remaining 999 client requests in an awaiting state and streams the origin's response back to all 1,000 clients simultaneously as bytes arrive.
   - Configured in Nginx via `proxy_cache_use_stale updating` and `proxy_cache_lock on`.
2. **Probabilistic Early Expiration (XFetch)**:
   - Instead of waiting for `TTL = 0`, background worker threads proactively refresh the cache with probability $P(\Delta t) \propto e^{-\beta \times \text{remaining\_time}}$, ensuring the cache is continuously replenished before expiry without blocking user requests.
3. **Stale-While-Revalidate (`Cache-Control: max-age=600, stale-while-revalidate=60`)**:
   - Instructs the CDN edge to instantly return the expired cached object to incoming users while asynchronously triggering an origin revalidation in the background.
