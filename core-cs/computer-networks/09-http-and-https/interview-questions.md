# HTTP & HTTPS: Top FAANG Interview Questions

### Q1: What is the exact difference between `PUT` and `PATCH` in RESTful API design?
**Answer**:
- **`PUT` (Complete Resource Replacement)**:
  - Idempotent by definition.
  - The client provides the **entire state** of the resource.
  - If a User resource has fields `{id: 1, name: "Alice", email: "alice@example.com", age: 30}`, and the client sends `PUT /users/1` with body `{"name": "Alice Bob"}`, all other fields (`email`, `age`) are either deleted or reset to null/defaults.
  - Calling `PUT` 5 times with the exact same payload leaves the resource in the exact same state.
- **`PATCH` (Partial Resource Modification)**:
  - Not inherently idempotent (though often implemented idempotently).
  - The client provides only the **delta/subset of fields** to update.
  - Sending `PATCH /users/1` with body `{"name": "Alice Bob"}` modifies only the `name` field, preserving `email` and `age` untouched.
  - Can represent non-idempotent operations (e.g., JSON Patch operations like `{"op": "add", "path": "/items", "value": "item3"}`).

---

### Q2: What is the difference between `502 Bad Gateway` and `504 Gateway Timeout`? How do you debug each in production?
**Answer**:
- **Architecture Context**: Both status codes are emitted by an intermediate reverse proxy (e.g. AWS ALB, Cloudflare, Nginx, Envoy) sitting in front of upstream application servers (e.g. Node.js, Go, Python microservices).
- **`502 Bad Gateway`**:
  - The proxy connected to the upstream application, but received an **invalid, malformed, or severed response**.
  - *Common Root Causes*:
    1. Upstream application process crashed (OOM killed, unhandled segmentation fault) while processing the request, closing the TCP connection abruptly without sending an HTTP response.
    2. Upstream process refused the connection (`ECONNREFUSED`) because the service was restarting or not listening on that port.
    3. Header size exceeded proxy limits (e.g. huge cookies exceeding Nginx `large_client_header_buffers`).
- **`504 Gateway Timeout`**:
  - The proxy successfully opened a TCP connection to the upstream application and sent the HTTP request, but the upstream application **took longer to reply than the proxy's configured read timeout** (e.g. proxy timeout is 30s, but upstream took 31s).
  - *Common Root Causes*:
    1. Slow or deadlocked database query holding the application thread.
    2. Upstream application thread pool starvation (all worker threads blocked).
    3. Slow third-party external API call without a timeout.

---

### Q3: Why did HTTP/2 introduce HPACK header compression instead of standard gzip / zlib compression?
**Answer**:
- **The Vulnerability of Gzip (CRIME Attack - 2012)**:
  - Standard LZ77 / DEFLATE compression searches for repeated byte strings across the entire message stream and replaces them with shorter pointer references.
  - When an attacker can inject user-controlled input into an HTTP request (e.g. a URL parameter) that is sent alongside a secret cookie in the same stream, the attacker can guess the cookie character-by-character:
    - If the guess matches the secret cookie, the compressed byte stream shrinks by 1 or 2 bytes due to compression deduplication.
    - If the guess is wrong, the compressed byte stream length stays larger.
  - By observing encrypted packet sizes across the wire, attackers could extract sensitive session authentication cookies in minutes!
- **The HPACK Solution (RFC 7541)**:
  - HPACK is specifically designed for HTTP headers. It does not compress arbitrary substrings across secrets and untrusted inputs.
  - Instead, it uses indexed lookup tables (Static Table of 61 common headers and a Dynamic Table of previously seen full header name-value pairs) and static Huffman coding.
  - Secrets cannot be leaked through character-by-character size side channels.
