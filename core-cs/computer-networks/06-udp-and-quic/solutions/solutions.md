# Solutions: UDP & QUIC (HTTP/3) Exercises

---

### Solution 1: Connection Migration & Path Validation

1. **TCP Fate**:
   - In TCP, a connection is permanently bound to the 4-tuple `(192.168.1.10, 49210, 198.51.100.2, 443)`.
   - When the client's IP changes to `100.64.45.12`, the server has no record of this new 4-tuple. If the client tries to send data with the old sequence numbers, the server replies with `RST`.
   - The video stream freezes, errors out, and must perform DNS, 3-way handshake, and TLS from scratch.

2. **QUIC Recognition**:
   - The packet header includes the **Destination Connection ID (`0x8a7b6c5d4e3f2a1b`)**.
   - The QUIC server inspects the CID in user space. It locates the existing connection state in its hash map, decrypts the packet using the established AEAD keys, and continues processing data immediately without tearing down the session.

3. **Amplification Risk & Path Validation**:
   - **Risk**: An attacker could spoof an incoming packet with Victim's IP and the valid CID, inducing the server to blast high-bandwidth video traffic at Victim (Denial of Service amplification).
   - **Path Validation**:
     - The server limits data sent to the new unvalidated address to at most $3\times$ the amount of data received from it.
     - The server sends a **`PATH_CHALLENGE`** frame containing an 8-byte unpredictable random payload.
     - The client must respond with a **`PATH_RESPONSE`** frame reflecting the exact payload.
     - Once validated, the server updates the path and unlocks full transmission bandwidth.

---

### Solution 2: Cumulative Handshake Latency

RTT = $80 \text{ ms}$. Transmission delay for 10 KB on 100 Mbps link:
$$D_{\text{trans}} = \frac{10,000 \times 8 \text{ bits}}{100 \times 10^6 \text{ bps}} = 0.0008 \text{ s} = 0.8 \text{ ms} \approx 1 \text{ ms}$$

1. **HTTP/1.1 over Plain TCP (No TLS)**:
   - 1 RTT (TCP 3-way handshake) + 1 RTT (HTTP GET + HTTP 200)
   - Total: $2 \times 80 = \mathbf{160 \text{ ms}}$

2. **HTTP/1.1 over TCP + TLS 1.2 (Cold)**:
   - 1 RTT (TCP handshake) + 2 RTT (TLS 1.2 handshake: ClientHello, ServerHello/Cert, KeyExchange) + 1 RTT (HTTP Request/Response)
   - Total: $4 \times 80 = \mathbf{320 \text{ ms}}$

3. **HTTP/2 over TCP + TLS 1.3 (Cold)**:
   - 1 RTT (TCP handshake) + 1 RTT (TLS 1.3 handshake with Diffie-Hellman key share) + 1 RTT (HTTP Request/Response)
   - Total: $3 \times 80 = \mathbf{240 \text{ ms}}$

4. **HTTP/3 over QUIC (Cold)**:
   - 1 RTT (Unified QUIC + TLS 1.3 Handshake) + 1 RTT (HTTP/3 Request/Response)
   - Total: $2 \times 80 = \mathbf{160 \text{ ms}}$ (Matches unencrypted HTTP/1.1, with full encryption!)

5. **HTTP/3 over QUIC (0-RTT Resumption)**:
   - 0 RTT (Client sends HTTP GET in first encrypted UDP packet) + 1 RTT (Server replies with HTTP 200)
   - Total: $1 \times 80 = \mathbf{80 \text{ ms}}$
   - **Speedup**: **4x faster than TLS 1.2** ($80 \text{ ms}$ vs $320 \text{ ms}$)!
