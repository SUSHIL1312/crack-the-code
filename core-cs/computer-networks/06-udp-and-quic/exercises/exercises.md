# Hands-On Exercises: UDP & QUIC (HTTP/3)

---

### Exercise 1: Connection Migration Protocol Trace
**Goal**: Trace how QUIC maintains a live connection across network transitions.

**Scenario**:
A user is on a live video stream via QUIC over home Wi-Fi (`192.168.1.10:49210`). The user walks out of range onto cellular 5G (`100.64.45.12:58100`).
- Server IP: `198.51.100.2:443`.
- QUIC Connection ID: `0x8a7b6c5d4e3f2a1b`.
1. Explain what happens to an active TCP connection when this IP shift occurs.
2. Explain how the QUIC server identifies the incoming packet from the new IP `100.64.45.12`.
3. What security risk does connection migration introduce (e.g. amplification attacks)? How does QUIC's **Path Validation (`PATH_CHALLENGE` / `PATH_RESPONSE`)** prevent it?

---

### Exercise 2: Cumulative Handshake Latency Modeling
**Goal**: Mathematically calculate the time required to fetch a 10 KB web page across different protocol generations.

**Parameters**:
- Client-Server RTT = $80 \text{ ms}$.
- Available Bandwidth = $100 \text{ Mbps}$.
- DNS lookup already cached in local memory.
- Page size = 10 KB (fits in a single initial congestion window).
- Calculate total elapsed time from clicking a link to the page being fully rendered for:
  1. HTTP/1.1 over Plain TCP (no TLS).
  2. HTTP/1.1 over TCP + TLS 1.2 (cold connection).
  3. HTTP/2 over TCP + TLS 1.3 (cold connection).
  4. HTTP/3 over QUIC (cold connection).
  5. HTTP/3 over QUIC (0-RTT connection resumption).
