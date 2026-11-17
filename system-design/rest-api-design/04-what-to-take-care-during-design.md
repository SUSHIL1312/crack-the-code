# 04: What to Take Care of During API Design (Production Checklist)

Designing a REST API for a hobby project is easy. Designing a REST API that runs at **Google, Stripe, Uber, or Netflix** scale requires handling edge cases: network timeouts, concurrent edits, double payments, database scale bottlenecks, and security.

Here is the **exhaustive production checklist** tested in FAANG interviews.

---

## 1. Idempotency Keys (Never Double-Charge a Customer!)

### 🚨 The Real-World Nightmare:
1. A customer taps **"Pay $100"** on their mobile phone.
2. The server successfully processes the credit card payment.
3. Suddenly, the user drives into a subway tunnel—their cellular connection drops before the server's `200 OK` response reaches their phone.
4. The mobile app shows a network timeout error. The customer taps **"Retry"**.
5. **Without Idempotency**: The customer gets charged $100 a second time!

### 💡 The Solution: The `Idempotency-Key` Header
When initiating mutating actions (`POST`), the client generates a unique UUID (e.g. `Idempotency-Key: 9b1deb4d-3b7d-4bad-9bdd-2b0d7b3dcb6d`) and sends it in the HTTP header:

```http
POST /v1/payments
Idempotency-Key: 9b1deb4d-3b7d-4bad-9bdd-2b0d7b3dcb6d
Content-Type: application/json

{
  "amount": 10000,
  "currency": "usd",
  "customer_id": "cus_9281"
}
```

### How the Server Handles It (Redis Atomic Lock Flow):
```
Client Request with Idempotency-Key
              │
              ▼
   Does Key exist in Redis?
         ├─── YES, status = "COMPLETED":
         │    └──► Return CACHED response immediately! (Do NOT re-charge card!)
         │
         ├─── YES, status = "IN_FLIGHT":
         │    └──► Return 409 Conflict ("Transaction currently processing, please wait")
         │
         └─── NO (First time seeing key):
              ├── 1. Acquire Redis lock with key and set status = "IN_FLIGHT"
              ├── 2. Process payment with payment gateway
              ├── 3. Save result in Database
              ├── 4. Store final response body in Redis (with 24h TTL)
              └── 5. Return 201 Created to client
```

![REST API Idempotency and Optimistic Concurrency Control Flow](../diagrams/rest_idempotency_locking.jpg)

---

## 2. Pagination: Offset vs Cursor (Keyset)

Never return an unbounded list (`SELECT * FROM orders`). You must paginate!

### Option A: Offset-Based Pagination (`?limit=20&offset=40`)
```http
GET /v1/orders?limit=20&offset=40
```
- **Pros**: Easy to implement; supports jumping to arbitrary page numbers (e.g. "Go to page 5").
- **Catastrophic Flaw 1 (Skipped/Duplicate records)**: If a user is viewing page 1, and a new order is inserted, all existing orders shift down by 1. When the user clicks page 2, they see the last item from page 1 again!
- **Catastrophic Flaw 2 (Database Performance)**: In SQL:
  ```sql
  SELECT * FROM orders ORDER BY id DESC LIMIT 20 OFFSET 1000000;
  ```
  The database engine must scan and discard **1,000,000 rows** from disk before returning the 20 rows! This causes database CPU spikes and timeouts.

### Option B: Cursor-Based Pagination (Industry Best Practice)
The client passes a **cursor** (an opaque pointer, often a base64-encoded ID or timestamp) representing the last seen item:

```http
GET /v1/orders?limit=20&cursor=eyJpZCI6IDUwMDF9
```
*Decoded cursor: `{"id": 5001}`*

The database executes an indexed range seek:
```sql
SELECT * FROM orders WHERE id < 5001 ORDER BY id DESC LIMIT 20;
```
- **Pros**:
  - Constant $O(\log N)$ performance via B-Tree index lookup, whether on item #10 or item #10,000,000.
  - Zero duplicate or skipped records even while new rows are being inserted continuously.
- **Response Format**:
```json
{
  "data": [ ... 20 orders ... ],
  "pagination": {
    "has_more": true,
    "next_cursor": "eyJpZCI6IDQ5ODF9"
  }
}
```

---

## 3. Standardized Error Format (RFC 7807 / RFC 9457 Problem Details)

Never return ad-hoc error strings like `{"error": "invalid"}` or raw internal stack traces.
Follow the standard **RFC 7807 Problem Details** format (`Content-Type: application/problem+json`):

```json
{
  "type": "https://api.example.com/errors/validation-failed",
  "title": "Validation Error",
  "status": 422,
  "detail": "The payload contains 2 invalid fields.",
  "instance": "/v1/orders",
  "invalid_params": [
    {
      "name": "quantity",
      "reason": "Must be greater than 0"
    },
    {
      "name": "email",
      "reason": "Invalid email address format"
    }
  ],
  "timestamp": "2026-11-17T02:30:00Z",
  "trace_id": "c7a82e9b-44f0-4a82"
}
```
**Why this matters**:
- Machine-readable (`type` and `invalid_params` allow frontend apps to highlight exact form fields).
- Debuggable (`trace_id` allows engineers to search distributed tracing logs in Datadog/Jaeger).

---

## 4. Concurrency Control & Preventing "Lost Updates" (ETags)

### 🚨 The "Lost Update" Problem:
1. Alice opens Order #42 at 10:00 AM (`status: "pending"`).
2. Bob opens Order #42 at 10:01 AM (`status: "pending"`).
3. Alice changes the address to **"New York"** and clicks Save at 10:02 AM.
4. Bob changes the status to **"shipped"** and clicks Save at 10:03 AM.
5. **Result**: Bob's save accidentally overwrites Alice's address update back to the old address! Alice's change is permanently lost.

### 💡 The Solution: Optimistic Locking with `ETag` and `If-Match`
1. When Alice fetches the order:
   ```http
   GET /v1/orders/42
   
   HTTP/1.1 200 OK
   ETag: "hash-version-101"
   { "id": 42, "address": "Chicago", "status": "pending" }
   ```
2. When Alice updates, she sends the ETag back:
   ```http
   PATCH /v1/orders/42
   If-Match: "hash-version-101"
   { "address": "New York" }
   
   HTTP/1.1 200 OK
   ETag: "hash-version-102"
   ```
3. When Bob attempts to update using his stale ETag:
   ```http
   PATCH /v1/orders/42
   If-Match: "hash-version-101"
   { "status": "shipped" }
   
   HTTP/1.1 412 Precondition Failed
   ```
   The server rejects Bob's update! Bob's client must re-fetch the latest data before applying changes.

---

## 5. Rate Limiting and Throttling

Protect your backend from DDoS, runaway bots, and noisy neighbors using the **Token Bucket** algorithm.

### Standard Response Headers:
Always return rate limit status in standard HTTP headers:
```http
HTTP/1.1 200 OK
RateLimit-Limit: 1000
RateLimit-Remaining: 994
RateLimit-Reset: 1700000000
```

When the client exceeds the quota:
```http
HTTP/1.1 429 Too Many Requests
Retry-After: 60
Content-Type: application/problem+json

{
  "type": "https://api.example.com/errors/rate-limit-exceeded",
  "title": "Too Many Requests",
  "status": 429,
  "detail": "Rate limit quota exceeded. Try again in 60 seconds."
}
```

---

## 6. Versioning Strategies

| Strategy | Syntax Example | Pros | Cons | Industry Adoption |
|---|---|---|---|---|
| **URI Path** | `/v1/users`, `/v2/users` | Explicit, easy to test in browsers, transparent routing. | Changes URI of the resource. | **Stripe, GitHub, Google Cloud** |
| **Custom Header** | `X-API-Version: 2` | Clean URIs. | Harder to test in browser address bar. | **Twilio** |
| **Accept Header (Content Negotiation)** | `Accept: application/vnd.app.v2+json` | Purest REST adherence. | High complexity for clients and proxy caching. | Enterprise internal APIs |

### 🎯 Golden Rule for Versioning:
- **Non-breaking change** (Add a new field, add a new endpoint): DO NOT bump version. Make fields optional or additive.
- **Breaking change** (Remove a field, rename a field, change data types, alter authentication): Release a new major version (`/v2/`).

---

## 7. Security Essentials Checklist

- [ ] **Authentication**: Pass Bearer token via `Authorization: Bearer <jwt>` header (Never in URL query strings where it gets logged in server access logs!).
- [ ] **Use Type-Prefixed IDs**: Instead of integer auto-increment IDs (`42`), use Stripe-style type-prefixed IDs (`usr_9a8f2`, `ord_7712a`) or UUIDv7. Prevents attackers from scraping all data by enumerating `id=1,2,3...`!
- [ ] **CORS (Cross-Origin Resource Sharing)**: Properly configure `Access-Control-Allow-Origin`, `Access-Control-Allow-Methods`, and handle `OPTIONS` preflight requests.
- [ ] **Input Validation & Sanitization**: Reject any unexpected fields (prevent Mass Assignment vulnerabilities) and sanitize input against injection.
