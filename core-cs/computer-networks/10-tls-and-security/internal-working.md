# Internal Working: TLS Handshake Wire Sequences, SNI, & HKDF

---

## 1. TLS 1.2 vs TLS 1.3 Handshake Wire Traces

### TLS 1.2 Handshake (2 RTTs):
```
Client                                                         Server
  |                                                              |
  |--- 1. ClientHello (Supported ciphers, TLS versions, SNI) --->|
  |                                                              |
  |<-- 2. ServerHello (Chosen cipher suite) ---------------------|
  |<-- 3. Certificate (Cleartext X.509 cert chain) --------------|
  |<-- 4. ServerKeyExchange (ECDHE parameters + signature) ------|
  |<-- 5. ServerHelloDone ---------------------------------------|
  |                                                              |
  |--- 6. ClientKeyExchange (Client ECDHE public key) ---------->|
  |--- 7. ChangeCipherSpec ------------------------------------->|
  |--- 8. Finished (Encrypted verification) -------------------->|
  |                                                              |
  |<-- 9. ChangeCipherSpec --------------------------------------|
  |<-- 10. Finished (Encrypted verification) --------------------|
  |                                                              |
  | <=================== Encrypted Application Data ===========> |
  [ Handshake took 2 full round trips before sending data! ]
```

### TLS 1.3 Handshake (1 RTT):
```
Client                                                         Server
  |                                                              |
  |--- 1. ClientHello ------------------------------------------>|
  |       + KeyShare (Client's ECDHE Public Key on X25519)       |
  |       + SupportedGroups (X25519, secp256r1)                  |
  |       + SNI (api.example.com)                                |
  |                                                              |
  |<-- 2. ServerHello -------------------------------------------|
  |       + KeyShare (Server's ECDHE Public Key)                 |
  |       [=== Shared Secret Derived! All following encrypted ===]
  |<-- 3. EncryptedExtensions (ALPN = h2) -----------------------|
  |<-- 4. Certificate (ENCRYPTED! Protected from eavesdroppers) -|
  |<-- 5. CertificateVerify (Digital signature over handshake) --|
  |<-- 6. Finished (Encrypted MAC) ------------------------------|
  |                                                              |
  |--- 7. Finished --------------------------------------------->|
  |--- 8. First Encrypted HTTP GET Data ------------------------>|
  |                                                              |
  [ Handshake finished in 1 single RTT! ]
```

---

## 2. Server Name Indication (SNI) & Encrypted Client Hello (ECH)

### Why SNI is Essential:
In modern cloud environments, a single physical IP address hosts thousands of independent websites (virtual hosting).
- When a client initiates a TLS connection to `api.example.com`, the TLS handshake occurs **before** the HTTP request is sent!
- The server doesn't know which domain the client wants, so it doesn't know which of its 1,000 SSL certificates to present.
- **SNI Extension (RFC 6066)**: The client specifies the hostname (`server_name: api.example.com`) in plaintext inside the initial `ClientHello`. The server inspects SNI and serves the correct certificate.

### Privacy Issue & Encrypted Client Hello (ECH / RFC 9460):
- Because SNI was historically unencrypted, local Wi-Fi eavesdroppers and ISPs could observe which websites users visited.
- **ECH**: Encrypts the entire ClientHello (including SNI) using a public key published in the domain's DNS `HTTPS` / `SVCB` resource record, restoring end-to-end domain privacy.

---

## 3. HKDF: HMAC-based Key Derivation Function (RFC 5869)

In TLS 1.3, all encryption keys are derived through a mathematically rigorous **Key Schedule** using HKDF:

```
                  Early Secret (derived from PSK in 0-RTT)
                            |
                     (ECDHE Shared Secret)
                            v
                  Handshake Secret
                  /              \
       Client Handshake Key     Server Handshake Key
       (Encrypts Handshake)     (Encrypts Handshake)
                            |
                            v
                  Master Secret
                  /              \
       Client Application Key   Server Application Key
       (Encrypts HTTP Data)     (Encrypts HTTP Data)
```
- Traffic keys in each direction are completely independent.
- Every session key is cryptographically bound to the entire transcript of all prior handshake messages, guaranteeing that any active Man-in-the-Middle tampering corrupts the keys and causes an immediate handshake failure.
