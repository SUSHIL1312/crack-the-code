# Solutions: TLS & Network Security Exercises

---

### Solution 1: OCSP Stapling Architecture

1. **Why Traditional Synchronous OCSP Fails**:
   - **Latency**: Every new HTTPS connection must execute an extra out-of-band HTTP request to the third-party CA's OCSP server, adding 50 to 300 ms of blocking latency before the TLS handshake can finish!
   - **Privacy Leak**: The CA's OCSP server sees every domain every user visits in real-time alongside the client's IP address.
   - **Availability**: If the CA's OCSP server experiences an outage or DDoS, the entire Internet cannot connect to legitimate websites (soft-fail renders it useless; hard-fail breaks connectivity).

2. **How OCSP Stapling Solves It**:
   - The **Web Server** periodically queries the CA's OCSP server in the background (e.g. once every hour).
   - The CA returns a **cryptographically signed and timestamped OCSP response** proving the certificate is not revoked.
   - When a client connects, the web server "staples" this cached, signed OCSP response directly inside the TLS `CertificateStatus` handshake message!
   - **Benefits**:
     - Client verifies revocation locally using the CA's signature in $0 \text{ ms}$ extra latency!
     - CA never sees the client's IP address (100% privacy preserved).
     - Survives CA outages because responses are valid for several days.

3. **Must-Staple Behavior**:
   - Without Must-Staple: If an attacker blocks the stapled response, the client falls back to soft-fail and accepts the connection.
   - With Must-Staple (X.509 extension `1.3.6.1.5.5.7.1.24`): The certificate explicitly demands a valid stapled OCSP response. If missing or expired, the browser **hard-fails and refuses connection**, preventing attackers from using revoked certificates!

---

### Solution 2: MitM on Plain Diffie-Hellman

1. **Alice's Key**:
   - Alice receives $M_2$ (from Mallor) and computes:
     $$S_{\text{Alice}} = (M_2)^a \pmod p = (g^{m_2})^a = \mathbf{g^{a \cdot m_2} \pmod p}$$

2. **Bob's Key**:
   - Bob receives $M_1$ (from Mallor) and computes:
     $$S_{\text{Bob}} = (M_1)^b \pmod p = (g^{m_1})^b = \mathbf{g^{b \cdot m_1} \pmod p}$$

3. **Mallor's Keys**:
   - Mallor knows $m_1$ and $m_2$. Mallor calculates:
     $$S_1 = A^{m_2} = g^{a \cdot m_2} = S_{\text{Alice}}$$
     $$S_2 = B^{m_1} = g^{b \cdot m_1} = S_{\text{Bob}}$$
   - **Mallor establishes two separate encrypted sessions!**
   - When Alice sends an encrypted message with $S_{\text{Alice}}$, Mallor decrypts it with $S_1$, inspects or alters the data, re-encrypts it with $S_2$, and forwards it to Bob. Bob decrypts with $S_{\text{Bob}}$ with zero errors!

4. **How TLS Digital Signatures Prevent It**:
   - In TLS, the server does **not** send an anonymous public key $B$.
   - The server signs its public key $B$ and the handshake transcript using the **private key matching its verified X.509 Certificate** (`CertificateVerify`).
   - Mallor cannot forge this digital signature without possessing the server's certified private key.
   - If Mallor replaces $B$ with $M_1$, the signature check fails, and the client aborts with `ERR_CERT_AUTHORITY_INVALID`!
