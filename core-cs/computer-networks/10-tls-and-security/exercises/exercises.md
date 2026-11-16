# Hands-On Exercises: TLS & Network Security

---

### Exercise 1: Certificate Expiration & OCSP Stapling Architecture
**Goal**: Design a high-availability certificate validation architecture for an API gateway serving 500,000 requests per second.

**Problem**:
A naive client validates certificate revocation by synchronously querying the CA's Online Certificate Status Protocol (OCSP) server (`http://ocsp.digicert.com`) for every incoming HTTPS connection.
1. Why does traditional synchronous OCSP lookup introduce a massive latency and privacy bottleneck?
2. How does **OCSP Stapling (RFC 6066)** solve both the latency penalty and the privacy leak?
3. What happens if the OCSP responder goes offline under OCSP Stapling with **Must-Staple** enabled vs disabled?

---

### Exercise 2: Tracing Man-in-the-Middle (MitM) Attacks on Plain Diffie-Hellman
**Goal**: Prove why Diffie-Hellman requires digital signature authentication.

**Scenario**:
Alice and Bob perform unauthenticated Diffie-Hellman key exchange. Mallor (active attacker on the router) intercepts all traffic between them:
1. When Alice sends public key $A = g^a \pmod p$, Mallor intercepts it and sends $M_1 = g^{m_1} \pmod p$ to Bob.
2. When Bob sends public key $B = g^b \pmod p$, Mallor intercepts it and sends $M_2 = g^{m_2} \pmod p$ to Alice.
3. Calculate the shared secret Alice computes.
4. Calculate the shared secret Bob computes.
5. Can Mallor decrypt, read, tamper with, and re-encrypt all messages between Alice and Bob without either detecting it?
6. How does TLS use X.509 digital signatures (`CertificateVerify`) to mathematically prevent this attack?
