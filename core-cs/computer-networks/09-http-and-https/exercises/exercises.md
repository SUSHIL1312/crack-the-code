# Hands-On Exercises: HTTP, HTTPS, & Web Protocols

---

### Exercise 1: Conditional Caching Architecture with `ETag`
**Goal**: Design an end-to-end caching mechanism using HTTP headers for a high-traffic REST endpoint `GET /api/v1/catalog`.

**Requirements**:
1. When a client requests `GET /api/v1/catalog` for the first time:
   - What headers should the server return to cache the resource for up to 10 minutes in intermediate CDN caches, require re-validation in browser caches, and provide an entity digest?
2. When the client makes a second request 2 minutes later:
   - What conditional header does the client include?
3. If the catalog data on the server has not changed:
   - What status code does the server return? Does it send a body?
4. If an administrator updates the catalog prices:
   - Trace the exact server response headers and status code.

---

### Exercise 2: Implementing Idempotent REST API Retries
**Goal**: Prevent duplicate payment charges across distributed microservices.

**Scenario**:
A mobile client calls `POST /api/v1/payments` with body `{"amount": 1000}`.
The server charges the user's credit card and updates the database, but before the HTTP 200 response reaches the client, the client's cellular connection drops.
The mobile app's automated network layer retries the HTTP request.
1. Why does a standard `POST` method cause duplicate payment processing upon retry?
2. How does introducing an **Idempotency Key (`Idempotency-Key: uuid-v4`)** in the header solve this distributed transaction problem?
3. What HTTP status code should the server return on the second duplicate request?
4. What happens if a second request arrives with the same Idempotency Key while the first request is still actively executing in the database?

---

### Exercise 3: HPACK Header Compression Savings
**Goal**: Calculate compression savings across 100 HTTP/2 requests.

**Data**:
A client makes 100 sequential requests to fetch image thumbnails on the same page.
Each request carries:
- `:method: GET`
- `:scheme: https`
- `:path: /images/thumb_N.jpg` (20 bytes)
- `:authority: static.example.com` (18 bytes)
- `user-agent: Mozilla/5.0...` (120 bytes)
- `cookie: session_id=xyz...` (80 bytes)
- Compare total header bytes transmitted under HTTP/1.1 (uncompressed plaintext) vs HTTP/2 HPACK (static table + dynamic table indexing).
