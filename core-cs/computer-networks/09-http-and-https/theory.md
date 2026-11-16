# Theoretical Foundations: HTTP, HTTPS, and Web Protocols

---

## 1. HTTP Protocol Generational Comparison

| Feature | HTTP/1.1 (RFC 7230 / 9112) | HTTP/2 (RFC 7540 / 9113) | HTTP/3 (RFC 9114) |
| :--- | :--- | :--- | :--- |
| **Transport Layer** | TCP | TCP | **QUIC (over UDP)** |
| **Data Format** | Plaintext ASCII | Binary Frames | Binary Frames |
| **Multiplexing** | No (Pipelining failed) | Yes (Multiplexed Streams over 1 TCP) | Yes (Multiplexed Streams over QUIC) |
| **Head-of-Line Blocking** | Application-Layer & Transport | **Transport-Layer only** (TCP packet drop stalls all) | **None** (Independent streams) |
| **Header Compression** | None (Redundant headers repeated) | **HPACK** (Static + Dynamic table) | **QPACK** (Optimized for out-of-order) |
| **Handshake Latency** | 1 RTT (TCP) + 1-2 RTT (TLS) = 2-3 RTT | 1 RTT (TCP) + 1 RTT (TLS 1.3) = 2 RTT | **1 RTT Initial / 0 RTT Resumption** |
| **Connection Migration** | Breaks on IP change | Breaks on IP change | **Seamless** (via Connection ID) |

---

## 2. RESTful Principles: Safety and Idempotency

In RESTful HTTP API design:
- **Safe Method**: A method that **does not alter server state** (read-only). Safe methods can be aggressively cached, prefetched, and bookmarked.
- **Idempotent Method**: A method where making $N$ identical requests produces the **exact same server state** as making 1 request ($f(f(x)) = f(x)$). Crucial for automatic network retry logic!

| HTTP Method | Safe? | Idempotent? | Typical Use Case |
| :--- | :--- | :--- | :--- |
| **`GET`** | **Yes** | **Yes** | Retrieve resource state. No side effects. |
| **`HEAD`** | **Yes** | **Yes** | Retrieve response headers only (no body). Check content length/existence. |
| **`OPTIONS`** | **Yes** | **Yes** | Query server capabilities (CORS preflight). |
| **`PUT`** | No | **Yes** | **Complete Replacement**: Replace resource with payload at exact URI. Sending 5 times results in the same state. |
| **`DELETE`** | No | **Yes** | Delete resource. Calling `DELETE /users/42` 5 times leaves user 42 deleted. |
| **`POST`** | No | **No** | **Append / Execute**: Creates a new subordinate resource or triggers an action. Sending 5 times creates 5 separate resources! |
| **`PATCH`** | No | **No** (unless designed) | **Partial Modification**: Modifies fields of an existing resource (e.g. `{"age": 30}`). |

---

## 3. HTTP Status Code Taxonomy

- **`2xx` Success**:
  - `200 OK`: Standard success with payload.
  - `201 Created`: Resource successfully created (`Location` header provides URI).
  - `204 No Content`: Action succeeded, no response body returned (common for `DELETE`).
- **`3xx` Redirection**:
  - `301 Moved Permanently`: Permanent redirect; search engines update index; browsers cache redirect.
  - `302 Found`: Temporary redirect (legacy, often incorrectly rewritten to GET).
  - `304 Not Modified`: Cached copy in client browser is still fresh (`ETag` matched); zero payload body transmitted!
  - `307 Temporary Redirect` & `308 Permanent Redirect`: Strict redirects preserving original HTTP method and body.
- **`4xx` Client Error**:
  - `400 Bad Request`: Malformed syntax or invalid validation.
  - `401 Unauthorized`: Authentication required (missing or invalid token/header).
  - `403 Forbidden`: Authenticated, but lacking permission/authorization to access resource.
  - `404 Not Found`: Target resource does not exist.
  - `409 Conflict`: Request conflicts with current state (e.g. concurrent edit, duplicate email).
  - `429 Too Many Requests`: Rate limit exceeded (`Retry-After` header specifies wait time).
- **`5xx` Server Error**:
  - `500 Internal Server Error`: Unhandled server-side exception or crash.
  - `502 Bad Gateway`: Reverse proxy (Nginx, ALB) received an invalid response or connection refusal from the upstream backend service.
  - `503 Service Unavailable`: Server is temporarily overloaded or down for maintenance.
  - `504 Gateway Timeout`: Reverse proxy timed out waiting for the upstream backend service to respond within the configured timeout window.

---

## 4. HTTP Caching & Conditional Requests

To eliminate redundant network data transfers:
1. **`Cache-Control`**:
   - `max-age=3600`: Cache fresh for 1 hour.
   - `no-cache`: Client may cache, but **must validate with server** before using!
   - `no-store`: Never write data to disk or cache (confidential/financial data).
   - `public` vs `private`: Can intermediate CDNs/proxies cache it, or only the end-user browser?
2. **Conditional Validation via `ETag` (Entity Tag)**:
   - Server returns: `ETag: "686897696a7c76"`.
   - On next request, client sends: `If-None-Match: "686897696a7c76"`.
   - If content is unchanged: Server replies **`304 Not Modified` with 0 bytes body**, saving bandwidth and CPU rendering time!
