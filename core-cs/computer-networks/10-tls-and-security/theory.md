# Theoretical Foundations: TLS 1.2 / 1.3 & Network Cryptography

---

## 1. Cryptographic Building Blocks of TLS

TLS achieves three core security properties:
1. **Confidentiality (Privacy)**: No eavesdropper can read the data. Achieved via **Symmetric Encryption** (AES-256-GCM, ChaCha20-Poly1305).
2. **Integrity (Tamper-Resistance)**: No adversary can alter data in transit without detection. Achieved via **AEAD (Authenticated Encryption with Associated Data)**.
3. **Authentication (Identity Verification)**: Proves that the server is truly the intended destination, not an impersonator. Achieved via **Asymmetric Digital Signatures (ECDSA, RSA)** and **X.509 PKI**.

### Symmetric vs Asymmetric Cryptography:
- **Asymmetric Cryptography** (RSA, ECC): Computationally expensive (~1,000x slower than symmetric). Used **only during the initial handshake** to authenticate the server and negotiate a secret key.
- **Symmetric Cryptography** (AES-GCM, ChaCha20): Hardware-accelerated (AES-NI CPU instructions can encrypt at > 10 GB/s per core). Used for all **bulk application data transfer**.

---

## 2. Diffie-Hellman Key Exchange & Perfect Forward Secrecy (PFS)

### Diffie-Hellman Mathematical Principle:
How do two strangers compute a shared secret in public without an eavesdropper learning it?

$$\text{Public Parameters: Generator } g, \text{ Prime modulus } p$$
1. **Client** picks private secret $a$, computes public key $A = g^a \pmod p$, sends $A$ to Server.
2. **Server** picks private secret $b$, computes public key $B = g^b \pmod p$, sends $B$ to Client.
3. **Shared Secret Derivation**:
   - Client calculates: $S = B^a \pmod p = (g^b)^a = g^{ab} \pmod p$.
   - Server calculates: $S = A^b \pmod p = (g^a)^b = g^{ab} \pmod p$.
   - Both sides now hold the identical secret $S = g^{ab} \pmod p$, while an eavesdropper observing only $A$ and $B$ cannot compute $S$ without solving the intractable **Discrete Logarithm Problem**!

### Why Static RSA Key Exchange was Deprecated in TLS 1.3:
- In legacy TLS (RSA key exchange): Client generated a Pre-Master Secret, encrypted it with the server's public RSA key, and sent it across the wire.
- **The Catastrophic Vulnerability**:
  - An adversary (e.g. intelligence agency or hacker) captures and records all encrypted traffic passing through an internet backbone for 5 years.
  - If the server's private RSA key is compromised, leaked, or stolen 5 years later, the adversary can decrypt **every single recorded historical session**!
- **Perfect Forward Secrecy (PFS) with ECDHE**:
  - In Ephemeral Diffie-Hellman (ECDHE), new temporary keys ($a$ and $b$) are generated for **every single session** and deleted from RAM immediately after session key derivation.
  - Even if the server's private certificate key is compromised in the future, past encrypted sessions **cannot be decrypted**!

---

## 3. The Public Key Infrastructure (PKI) Chain of Trust

How does a browser know that the server sending an ECDHE public key is really `google.com` and not an attacker on the same Starbucks Wi-Fi?

```
[ Root Certificate Authority (e.g. DigiCert Global Root CA) ]
  - Pre-installed in operating system / browser Trust Store (hardware root anchor)
  - Signs Intermediate CA certificate with Root Private Key
        |
        v
[ Intermediate Certificate Authority (e.g. DigiCert TLS RSA SHA256 2020 CA1) ]
  - Signs Leaf / End-Entity Certificate with Intermediate Private Key
        |
        v
[ Leaf Certificate (*.google.com) ]
  - Contains: Public Key of google.com, Domain Names (SAN), Validity Dates
  - Signed by: Intermediate CA
```

- When the browser connects, the server sends its Leaf Certificate and the Intermediate Certificate.
- The browser walks the chain up to its local OS **Trust Store** and verifies each digital signature mathematically using the issuer's public key.

---

## 4. The Revolutionary Leap: TLS 1.2 vs TLS 1.3 (RFC 8446)

| Dimension | TLS 1.2 (2008) | TLS 1.3 (2018) |
| :--- | :--- | :--- |
| **Handshake Latency** | **2 Full RTTs** | **1 Single RTT** (or 0 RTT resumption) |
| **Key Exchange** | Static RSA, Static DH, or ECDHE | **ECDHE only** (Mandatory Perfect Forward Secrecy) |
| **Cipher Suites Supported** | ~300 (included insecure RC4, 3DES, CBC) | **Only 5 ultra-secure AEAD suites** (e.g. `TLS_AES_256_GCM_SHA384`) |
| **Vulnerable Primitives** | Allowed MD5, SHA-1, RSA PKCS#1 v1.5 | **Completely purged** |
| **Handshake Encryption** | Certificate transmitted in cleartext! | **Certificate and server identity encrypted** |

In TLS 1.3, the client makes an educated guess about the server's supported curve (e.g. `X25519`) and sends its **Diffie-Hellman Key Share in the very first ClientHello**! The server replies with its key share in `ServerHello`, establishing symmetric encryption immediately in just **1 RTT**.
