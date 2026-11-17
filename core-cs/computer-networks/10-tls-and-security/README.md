# Module 10: Transport Layer Security (TLS 1.2 / 1.3) & Cryptography

<!-- Chapter Navigation Header -->
| ⬅️ [Module 09: HTTP & HTTPS](../09-http-and-https/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Module 11: Network Performance](../11-network-performance/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## Overview
Transport Layer Security (TLS, RFC 8446) is the cryptographic protocol that turns insecure plaintext internet channels into confidential, tamper-proof, and authenticated connections (HTTPS). Every secure web visit, mobile API call, database connection, and encrypted microservice RPC relies on TLS to defend against eavesdropping, data alteration, and impersonation.

In this module, you will master symmetric vs asymmetric cryptography, Elliptic-Curve Diffie-Hellman Ephemeral (ECDHE) key exchange, the X.509 Certificate Chain of Trust, Perfect Forward Secrecy (PFS), the evolutionary leap from TLS 1.2 to TLS 1.3, Server Name Indication (SNI), and Application-Layer Protocol Negotiation (ALPN).

---

## Learning Objectives
1. **Cryptographic Primitives**: Contrast Symmetric Ciphers (AES-GCM, ChaCha20-Poly1305) with Asymmetric Cryptography (RSA, ECC, ECDSA) and Cryptographic Hashes (SHA-256).
2. **Diffie-Hellman Key Exchange (ECDHE)**: Understand how two endpoints derive a shared secret over an unencrypted, monitored network wire.
3. **Perfect Forward Secrecy (PFS)**: Learn why TLS 1.3 completely banned static RSA key exchanges to protect historical traffic from future private key compromises.
4. **X.509 Public Key Infrastructure (PKI)**:
   - Root CAs, Intermediate CAs, and Leaf Certificates.
   - Certificate validation algorithms and Subject Alternative Names (SAN).
   - Revocation strategies: CRLs vs OCSP vs OCSP Stapling.
5. **TLS 1.2 vs TLS 1.3 Handshake**: Compare the 2-RTT TLS 1.2 handshake with the streamlined 1-RTT (and 0-RTT) TLS 1.3 key share architecture.
6. **SNI & ALPN**: Master multi-tenant virtual hosting via Server Name Indication (SNI) and protocol negotiation via ALPN (`h2`, `http/1.1`).

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of TLS 1.2/1.3, X.509 PKI, certificate validation, and cipher suites.
- `internal-working.md`: Handshake message sequence, Diffie-Hellman mathematical formulation, and key derivation schedule (HKDF).
- `visual-explanation.md`: Visual sequence comparisons of TLS 1.2 vs TLS 1.3 handshakes and certificate trust chains.
- `examples/`:
  - `tls_handshake_simulator.cpp`: Complete C++ cryptographic simulator demonstrating Diffie-Hellman key derivation and AES session key generation.
- `exercises/`: Certificate chain verification and cipher suite evaluation exercises.
- `solutions/`: Reference solutions.
- `interview-questions.md`: FAANG interview questions (e.g. "What is Perfect Forward Secrecy?", "How does SNI enable multiple SSL sites on one IP?").
- `common-mistakes.md`: Expired certificate outages, missing intermediate certificates, disabling certificate validation in code (`CURLOPT_SSL_VERIFYPEER = 0`).

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 09: HTTP & HTTPS](../09-http-and-https/README.md) | [Computer Networks Hub](../README.md) | **[Module 11: Network Performance](../11-network-performance/README.md)** |
<!-- /Chapter Navigation Footer -->

