# TLS & Network Security: Top FAANG Interview Questions

### Q1: What is Perfect Forward Secrecy (PFS), and why does TLS 1.3 make it mandatory?
**Answer**:
- **Definition**: Perfect Forward Secrecy is a cryptographic property ensuring that the compromise of a server's long-term private key does **not** compromise the confidentiality of previously recorded encrypted sessions.
- **The Threat**: An adversary captures and stores petabytes of encrypted HTTPS traffic over years. If the server used static RSA key exchange, obtaining the server's private key (via subpoena, employee bribe, server seizure, or Heartbleed memory leak) allows the adversary to decrypt every single past session recorded over those years.
- **How PFS Works**:
  - In Ephemeral Diffie-Hellman (ECDHE), the session keys are derived from **temporary (ephemeral) key pairs** generated dynamically in RAM for that single connection.
  - As soon as the session keys are computed, the ephemeral private keys are erased from memory.
  - Even if an attacker steals the server's master certificate private key tomorrow, the historical ephemeral keys no longer exist anywhere on Earth. The recorded ciphertexts remain computationally impossible to decrypt.
- **TLS 1.3 Decision**: TLS 1.3 completely removed static RSA and non-PFS cipher suites from the specification.

---

### Q2: What is ALPN (Application-Layer Protocol Negotiation) and how does it prevent extra round trips when upgrading to HTTP/2?
**Answer**:
- **The Dilemma**: When a client connects to `https://example.com:443`, the server could support HTTP/1.1 or HTTP/2.
- **Legacy Approach (NPN / Upgrade header)**:
  - Client performs full TLS handshake.
  - Client sends plaintext `GET / HTTP/1.1` with header `Upgrade: h2c`.
  - Server replies `101 Switching Protocols` to switch to HTTP/2.
  - Incurred extra round trips and failed behind non-compliant proxies.
- **The ALPN Solution (RFC 7301)**:
  - During the TLS handshake, the client includes the **`ALPN` extension** inside the initial `ClientHello`:
    `ALPN: [h2, http/1.1]`
  - The server selects its preferred protocol and confirms it in `EncryptedExtensions` inside the same handshake:
    `ALPN: h2`
  - When the TLS handshake completes, both client and server immediately start transmitting HTTP/2 binary frames on the very first byte with **zero additional round trips**!

---

### Q3: Why do browsers display an "Incomplete Certificate Chain" error on Android or Linux, even when the certificate works fine on macOS and Windows?
**Answer**:
- **Root Cause**: An X.509 certificate chain typically consists of:
  `Root CA -> Intermediate CA -> Leaf Certificate`
- **The Server Misconfiguration**:
  - The web server administrator only configured the Leaf certificate in Nginx (`ssl_certificate leaf.crt;`) and forgot to append the **Intermediate CA certificate** (`fullchain.pem`).
- **Why It Works on Mac/Windows**:
  - macOS (Keychain) and Windows (CryptoAPI) have built-in **AIA (Authority Information Access) fetching**: when an intermediate CA is missing, the OS automatically downloads it in the background from the CA's HTTP URL, or uses an intermediate certificate cached from a previous website visit.
- **Why It Fails on Linux/Android**:
  - Android and many Linux tools (curl, OpenSSL) strictly enforce RFC standards and **do not perform AIA fetching**. If the server fails to provide the intermediate certificate, the chain is broken, throwing `SEC_ERROR_UNKNOWN_ISSUER` or `x509: certificate signed by unknown authority`.
- **Fix**: Always configure the server with the full certificate bundle containing `Leaf + Intermediates` in order.
