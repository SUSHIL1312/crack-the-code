# Common Mistakes in the Domain Name System (DNS)

### 1. Forgetting the Trailing Dot in DNS Zone Files
- **Mistake**: Writing `www IN CNAME example.com` in a BIND or DNS zone file without a trailing dot.
- **Reality**: In DNS zone syntax, any hostname lacking a trailing dot is interpreted as **relative to the origin zone**!
  - If the zone is `example.com`, writing `www IN CNAME example.com` actually creates:
    `www.example.com. IN CNAME example.com.example.com.`!
- **Fix**: Always include the trailing dot for fully qualified domain names: `www IN CNAME example.com.`.

---

### 2. Believing Changing an A Record Takes Effect Immediately
- **Mistake**: Changing an `A` record from `Old_IP` to `New_IP` during a production deployment and shutting down `Old_IP` 10 minutes later.
- **Reality**: If the old record had a TTL of 86,400 seconds (24 hours), millions of recursive resolvers and browser caches globally will continue sending customer traffic to `Old_IP` for the next 24 hours. Shutting down the old server prematurely causes catastrophic downtime for a large portion of your user base.
- **Fix**: Lower the TTL to 300 seconds at least 48 hours before planned migrations.

---

### 3. Blocking the Event Loop with Synchronous DNS Calls
- **Mistake**: Calling POSIX `getaddrinfo()` directly inside a single-threaded non-blocking event loop (like Node.js, libuv, or C++ epoll server).
- **Reality**: Standard POSIX `getaddrinfo()` is a **blocking synchronous system call**. If upstream DNS is slow or dropping packets, `getaddrinfo()` can block the thread for up to 5 to 30 seconds! In a single-threaded server, this freezes all other concurrent user connections on the event loop.
- **Fix**: Offload DNS resolution to a background thread pool (how Node.js / libuv does it) or use an asynchronous non-blocking DNS library like `c-ares`.

---

### 4. Overlooking Negative Caching (RFC 2308)
- **Mistake**: Testing a new subdomain before configuring its DNS record, getting an `NXDOMAIN` error, then creating the record and wondering why your browser still says the domain doesn't exist.
- **Reality**: Resolvers perform **Negative Caching**: they cache the `NXDOMAIN` failure for the duration of the zone's `SOA` minimum TTL (typically 1 hour to 1 day!).
- **Fix**: Flush your local DNS cache (`dscacheutil -flushcache` on Mac, `systemd-resolve --flush-caches` on Linux) or query authoritative nameservers directly with `dig @ns1.example.com subdomain.example.com`.
