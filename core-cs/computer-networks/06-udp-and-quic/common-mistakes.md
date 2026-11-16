# Common Mistakes in UDP and QUIC

### 1. Believing QUIC is Unreliable Because It Uses UDP
- **Mistake**: Assuming that because UDP is best-effort and unreliable, applications running over QUIC will experience packet loss and data corruption.
- **Reality**: UDP is merely the link-layer packaging vehicle for QUIC. The QUIC protocol library itself implements complete, rigorous reliability: packet acknowledgments, packet retransmission, dynamic RTO estimation, and congestion control (Cubic/BBR). QUIC is just as reliable as TCP, but without TCP's architectural constraints.

---

### 2. Assuming 0-RTT Early Data is Safe for All API Requests
- **Mistake**: Allowing clients to send financial transactions (`POST /api/pay?cents=5000`) in QUIC 0-RTT early data.
- **Reality**: 0-RTT data is susceptible to replay attacks by malicious network observers. An attacker capturing the packet can replay it 100 times, causing the server to process the payment 100 times. Never execute non-idempotent or state-changing operations in 0-RTT data.

---

### 3. Confusing HTTP/2 Stream Multiplexing with QUIC Stream Multiplexing
- **Mistake**: Believing that HTTP/2 eliminated Head-of-Line blocking.
- **Reality**: HTTP/2 eliminated **application-layer** HoL blocking (multiple requests per connection), but **exacerbated transport-layer** HoL blocking because all streams share a single sequential TCP byte stream. Only HTTP/3 (QUIC) eliminated transport-layer HoL blocking by making streams independent at Layer 4.

---

### 4. Overlooking UDP Throttling by Enterprise Firewalls
- **Mistake**: Deploying an HTTP/3-only public service without TCP fallback.
- **Reality**: Some corporate networks, university campuses, and legacy enterprise firewalls block outbound UDP on port 443 (due to historical suspicion of UDP abuse).
- **Fix**: Browsers always connect to HTTP/2 over TCP first, discover the server's HTTP/3 support via the `Alt-Svc: h3=":443"` header, and upgrade to QUIC opportunistically while preserving seamless TCP fallback.
