# REST API Design: FAANG Interview Questions & Answers

### Q1: What is the fundamental difference between `PUT` and `PATCH`, and what are the implications of choosing one over the other?
**Answer**:
- **`PUT` (Complete Replacement)**: Semantically replaces the target resource entirely with the provided payload. If fields are omitted from the request body, the server treats them as intentionally deleted or reset to their default values. `PUT` is strictly **idempotent**.
- **`PATCH` (Partial Modification)**: Applies a set of changes (a delta) to the existing resource. Only the fields present in the payload are modified; all unmentioned fields remain untouched.
- **Trade-offs**:
  - `PUT` requires the client to fetch the full resource first before modifying, which risks race conditions unless protected by `ETag` and `If-Match`.
  - `PATCH` saves bandwidth and enables fine-grained partial updates. However, defining how to remove fields or modify arrays requires choosing between **JSON Merge Patch (RFC 7396)** (setting a field to `null` deletes it) or **JSON Patch (RFC 6902)** (explicit `add`, `remove`, `replace` operations).

---

### Q2: Why is cursor-based pagination preferred over offset-based pagination in high-scale systems?
**Answer**:
1. **Database Query Cost ($O(1)$ vs $O(N)$)**:
   - Offset pagination (`LIMIT 20 OFFSET 1000000`) forces the database storage engine to sequentially scan and discard 1,000,000 index entries or rows from disk before returning 20 records.
   - Cursor-based pagination (`WHERE id < last_seen_id ORDER BY id DESC LIMIT 20`) performs a direct B-Tree index seek in $O(\log N)$ time, maintaining sub-millisecond response times regardless of depth.
2. **Data Consistency (Window Drift)**:
   - With offset pagination, if new rows are inserted while a user navigates between pages, all rows shift downward, causing the user to see duplicate records on subsequent pages.
   - Cursor pagination anchors the query to a deterministic unique key, guaranteeing no duplicate or skipped items even during continuous heavy ingestion.

---

### Q3: How do you design a payment checkout API to guarantee that a customer is never double-charged during network failures?
**Answer**:
1. **Mandate an `Idempotency-Key` Header**: Require the client to generate a unique UUID for each distinct checkout attempt and pass it in the `Idempotency-Key` header.
2. **Distributed Atomic Check & Lock**:
   - The API gateway or application server checks a distributed cache (Redis) for the idempotency key using `SET key token NX EX 86400`.
   - If the key is already marked `IN_PROGRESS`, return `409 Conflict` (or wait and poll).
   - If the key is marked `COMPLETED`, retrieve the previously cached HTTP status code, headers, and response body from Redis, returning it immediately without calling the payment gateway.
3. **Execution & Caching**:
   - If the key is new, acquire the lock, call the payment gateway, write the order to the database within a database transaction, and store the resulting HTTP response in Redis before returning `201 Created` to the client.

---

### Q4: Explain how `ETags` prevent the "Lost Update" problem in concurrent REST API systems.
**Answer**:
- The "Lost Update" problem occurs when Client A and Client B both read resource version 1. Client A updates and saves version 2. Client B, unaware of A's change, updates and saves version 3, accidentally overwriting Client A's modifications.
- **Solution using Optimistic Locking**:
  1. The server generates an `ETag` header (e.g. SHA-256 hash or database row version/timestamp) when returning a resource in `GET`:
     `ETag: "v1-hash"`.
  2. When a client submits a modifying request (`PUT` or `PATCH`), it must include the header `If-Match: "v1-hash"`.
  3. The server checks the current resource's ETag against the header value:
     - If they match, the update is applied, and a new ETag is generated.
     - If they do NOT match (meaning another client modified the resource in the meantime), the server aborts the transaction and returns `412 Precondition Failed`.
  4. The rejected client must re-fetch the latest resource representation and merge changes.

---

### Q5: How do you handle API versioning without breaking existing mobile clients that cannot be force-updated?
**Answer**:
1. **Additive (Non-Breaking) Changes**:
   - Never remove or rename existing fields in response payloads.
   - New functionality should add new optional fields or new endpoints.
   - Treat unknown fields leniently on clients (ignore unknown JSON keys).
2. **URI Path Versioning (`/v1/`, `/v2/`)**:
   - When breaking changes are unavoidable (e.g. data type change or architectural rewrite), release a new version prefix `/v2/`.
   - Maintain the `/v1/` routing layer through an API gateway (e.g. Envoy/Kong) that proxies requests or translates them to internal services.
   - Set an explicit deprecation timeline with standard headers: `Sunset: Wed, 11 Nov 2026 00:00:00 GMT` and `Deprecation: true`.
