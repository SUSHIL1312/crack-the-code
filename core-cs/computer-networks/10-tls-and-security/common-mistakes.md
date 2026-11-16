# Common Mistakes in TLS & Network Security

### 1. Disabling Certificate Validation in Production Code
- **Mistake**: Writing `curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L)` or setting `NODE_TLS_REJECT_UNAUTHORIZED = '0'` or Python `verify=False` to bypass an SSL error in development, then pushing to production.
- **Reality**: Disabling certificate verification completely negates the authentication guarantees of TLS. Any attacker on the local network (e.g. coffee shop Wi-Fi or compromised router) can perform a trivial Man-in-the-Middle attack with a self-signed certificate and decrypt all customer passwords, API tokens, and payment data in cleartext!
- **Fix**: Never disable verification in production. Install the internal corporate CA certificate into the system trust store instead.

---

### 2. Failing to Provide the Full Certificate Chain
- **Mistake**: Serving only the domain leaf certificate (`cert.pem`) instead of the full chain (`fullchain.pem`).
- **Reality**: The connection appears to work on modern macOS/Windows desktop browsers (which cache or auto-fetch intermediates), but fails on mobile Android clients, embedded IoT devices, and backend curl/Go services with `unable to get local issuer certificate`.
- **Fix**: Always configure web servers and load balancers with the full certificate bundle containing the leaf certificate followed immediately by the intermediate certificates.

---

### 3. Relying on Static RSA Cipher Suites
- **Mistake**: Configuring legacy cipher suites like `TLS_RSA_WITH_AES_256_CBC_SHA`.
- **Reality**: Static RSA lacks Perfect Forward Secrecy (PFS). If your private key is compromised in the future, all past recorded traffic can be decrypted retroactively. Furthermore, CBC mode is vulnerable to padding oracle attacks (e.g. Lucky Thirteen, POODLE).
- **Fix**: Restrict server configuration to modern AEAD cipher suites with ECDHE: `ECDHE-ECDSA-AES256-GCM-SHA384`, `ECDHE-RSA-AES128-GCM-SHA256`, or migrate directly to TLS 1.3.

---

### 4. Overlooking SNI Mismatch on Multi-Tenant IP Addresses
- **Mistake**: Connecting to an HTTPS server via raw IP address (`https://198.51.100.1`) when the IP hosts multiple SSL domains.
- **Reality**: Without a hostname, the client cannot populate the Server Name Indication (SNI) extension in `ClientHello`. The server falls back to its default certificate, triggering an `ERR_CERT_COMMON_NAME_INVALID` error because the certificate domain does not match the IP.
