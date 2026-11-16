# Solutions: HTTP, HTTPS, & Web Protocols Exercises

---

### Solution 1: Conditional Caching Architecture

1. **Initial Request Headers Returned by Server**:
   ```http
   HTTP/1.1 200 OK
   Content-Type: application/json
   Cache-Control: public, max-age=600, no-cache
   ETag: W/"v1-catalog-hash-8f4b12"
   Last-Modified: Sat, 16 Nov 2024 12:00:00 GMT
   Content-Length: 45000

   { ... 45 KB JSON catalog payload ... }
   ```
   - `public, max-age=600`: CDN caches the response for 10 minutes (600s).
   - `no-cache`: Instructs the client browser that it must send a conditional validation request to the server before rendering the cached copy.

2. **Second Request (Client Conditional Headers)**:
   ```http
   GET /api/v1/catalog HTTP/1.1
   Host: api.example.com
   If-None-Match: W/"v1-catalog-hash-8f4b12"
   If-Modified-Since: Sat, 16 Nov 2024 12:00:00 GMT
   ```

3. **Data Unchanged (Server Response)**:
   ```http
   HTTP/1.1 304 Not Modified
   ETag: W/"v1-catalog-hash-8f4b12"
   Cache-Control: public, max-age=600, no-cache
   ```
   - **Body**: Exactly **0 bytes transmitted**! The client renders its local cached copy immediately.

4. **Data Changed (Catalog Price Updated)**:
   ```http
   HTTP/1.1 200 OK
   ETag: W/"v2-catalog-hash-99a77c"
   Content-Type: application/json
   Content-Length: 45120

   { ... updated 45 KB payload ... }
   ```

---

### Solution 2: Idempotency Key Architecture for APIs

1. **Why Standard `POST` Causes Duplicates**:
   - `POST` is non-idempotent by definition. The server cannot know whether the second incoming request is a network retry of the first failed response, or an intentional second payment by the user.

2. **Idempotency Key Mechanism (Stripe / Uber Architecture)**:
   - Client generates a UUID v4: `Idempotency-Key: 9b1deb4d-3b7d-4bad-9bdd-2b0d7b3dcb6d`.
   - When the server receives the request:
     1. In an atomic transaction (e.g. Redis `SET key value NX EX 86400` or SQL table with unique constraint on `idempotency_key`), the server records the key with status `IN_PROGRESS`.
     2. If the key already exists and has status `COMPLETED`: The server **bypasses payment processing entirely** and immediately replays the cached HTTP response payload from the first execution!
3. **Status Code**:
   - Returns **`200 OK`** (or `201 Created`) with the exact same response body and payment ID as the first request. The client does not know or care that it was a retry; from the client's perspective, the operation succeeded.
4. **Concurrent In-Flight Retry**:
   - If a duplicate request arrives while the first is still executing (`IN_PROGRESS`), the server returns **`409 Conflict`** with an error message: `"A request with this idempotency key is currently processing. Please retry in a few seconds."`
