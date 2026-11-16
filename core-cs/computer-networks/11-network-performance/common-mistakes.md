# Common Mistakes: Network Performance & Optimization

---

### 1. Blindly Setting Massive `SO_RCVBUF` and `SO_SNDBUF` Values
- **The Mistake**: Developers trying to achieve maximum throughput on high-bandwidth links manually invoke:
  ```cpp
  int buf_size = 32 * 1024 * 1024; // 32 MB
  setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
  ```
- **Why It Fails**:
  1. Manual invocation of `setsockopt(SO_RCVBUF)` **disables Linux TCP window auto-tuning** (`tcp_moderate_rcvbuf`) for that socket.
  2. Linux locks that specific memory allocation permanently for the socket's lifetime, even when the socket is idle.
  3. If the server scales to 1,000 concurrent sockets, $1{,}000 \times 32\text{ MB} = 32\text{ GB}$ of un-reclaimable RAM is consumed, leading to catastrophic kernel `OOM` crashes.
- **The Fix**: Leave `SO_RCVBUF` unset and let the Linux kernel dynamically auto-tune buffer sizing via `net.ipv4.tcp_rmem` and `net.ipv4.tcp_wmem`.

---

### 2. Disabling TCP Window Scaling or Forgetting RFC 7323
- **The Mistake**: Configuring legacy firewalls, middleboxes, or custom OS kernels with `net.ipv4.tcp_window_scaling = 0`.
- **Why It Fails**:
  - Without window scaling, the 16-bit window field in the TCP header caps advertised window size at $2^{16} - 1 = 65{,}535\text{ bytes}$ (64 KB).
  - On any modern link with an RTT over 10 ms (e.g. cross-country or inter-datacenter), max throughput is hard-capped at $\approx 5\text{ Mbps}$, completely wasting modern 10 Gbps and 100 Gbps fiber pipes.
- **The Fix**: Ensure window scaling is enabled system-wide (`sysctl net.ipv4.tcp_window_scaling=1`).

---

### 3. Ignoring the Impact of Path MTU and Jumbo Frames
- **The Mistake**: Enabling 9000-byte Jumbo Frames on edge servers or internet-facing endpoints.
- **Why It Fails**:
  - Standard internet routers support a maximum transmission unit (MTU) of 1500 bytes.
  - If a server transmits 9000-byte frames across the public internet with the `DF` (Don't Fragment) bit set, intermediate routers drop the packet and return ICMP type 3 code 4 ("Fragmentation Needed and DF set").
  - If intermediate firewalls block ICMP packets (a common security anti-pattern), the sender experiences **PMTU Black Hole**: TCP handshakes succeed (small SYN packets fit in 1500 bytes), but data transfers stall indefinitely as soon as full-size packets are transmitted.
- **The Fix**: Use Jumbo Frames strictly within controlled datacenter fabrics (VPC internal subnets, SAN/NAS networks); keep internet egress at 1500 bytes (or 1420 bytes in VPN/overlay tunnels to account for GRE/Geneve/VXLAN encapsulation overhead).

---

### 4. Overlooking Cache Stampede (Thundering Herd) during CDN Purges
- **The Mistake**: Issuing an immediate wildcard purge (`PURGE /*`) across global CDN edge nodes after a deployment.
- **Why It Fails**:
  - Hundreds of thousands of concurrent client requests suddenly experience simultaneous cache misses across all global edge PoPs.
  - The CDN forwards all misses upstream simultaneously, hitting origin databases and backend microservices with massive multi-gigabit traffic spikes, taking down the entire backend infrastructure.
- **The Fix**:
  - Use **soft purging** / `stale-while-revalidate` so stale content is served while origin caches warm up.
  - Enable **origin request collapsing** (`proxy_cache_use_stale updating`, `proxy_cache_lock`).
  - Warm origin caches staging pre-deployment before rolling out CDN purge commands.
