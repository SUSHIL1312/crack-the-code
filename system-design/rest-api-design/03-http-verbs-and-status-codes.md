# 03: HTTP Verbs and Status Codes Mastery

<!-- Chapter Navigation Header -->
| ⬅️ [02: Resource Modeling & URIs](02-resource-modeling-and-uris.md) | 🏠 [REST API Design Hub](README.md) | [04: Production Checklist](04-what-to-take-care-during-design.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. Safety vs Idempotency: The Core Mathematical Principles
In API design interviews, interviewers frequently probe your understanding of **Safe** and **Idempotent** methods:

### What is a "Safe" Method?
A method is **Safe** if invoking it does NOT alter the observable state of the resource on the server.
* Safe: `GET`, `HEAD`, `OPTIONS`. (Reading data does not change data).

### What is an "Idempotent" Method?
In mathematics, an operation is idempotent if applying it multiple times produces the exact same state as applying it once:
$$f(f(x)) = f(x)$$
In HTTP, an idempotent method means: Making 1 request or making 10 identical requests results in the **exact same final server state**.
* If a network drops after sending a `PUT` or `DELETE` request, the client can safely retry the request without worrying about unintended side effects.

---

### HTTP Methods Matrix:

| HTTP Verb | CRUD Action | Safe? | Idempotent? | Typical Request Body | Typical Response Status |
|---|---|---|---|---|---|
| **`GET`** | Read | **YES** | **YES** | None | `200 OK` |
| **`POST`** | Create | **NO** | **NO** | Resource data | `201 Created` with `Location` header |
| **`PUT`** | Full Replace | **NO** | **YES** | Complete resource representation | `200 OK` or `204 No Content` |
| **`PATCH`** | Partial Update | **NO** | **NO / YES\*** | Delta of modified fields | `200 OK` |
| **`DELETE`** | Remove | **NO** | **YES** | None | `200 OK` or `204 No Content` |
| **`HEAD`** | Read Headers Only | **YES** | **YES** | None | `200 OK` (No body) |
| **`OPTIONS`**| Inspect Capabilities| **YES** | **YES** | None | `204 No Content` |

*\*Note on PATCH idempotency*: A PATCH that sets `"status": "active"` is idempotent. A PATCH that performs relative operations like `"balance += 10"` is non-idempotent.

---

## 2. PUT vs PATCH: The Vital Difference

### `PUT` = Complete Replacement
When you call `PUT /users/42`, you must send the **entire** updated resource. Any field you omit is treated as intentionally deleted or reset to null/default!

```http
PUT /users/42
Content-Type: application/json

{
  "name": "Alice Smith",
  "email": "alice@example.com",
  "phone": "555-0199"
}
```
*If you omit `"phone"`, the server will erase Alice's phone number!*

### `PATCH` = Partial Modification (Delta)
`PATCH` only modifies the specific fields provided in the payload. All unmentioned fields remain untouched:

```http
PATCH /users/42
Content-Type: application/json

{
  "email": "alice.new@example.com"
}
```
*Alice's name, phone, and password hash remain completely intact.*

---

## 3. Status Codes: When and Why to Use Each

A good REST API communicates meaning through HTTP status codes, not by returning `200 OK` with `{"error": "something failed"}`.

### 🟢 2xx Success:
- **`200 OK`**: The request succeeded. Returned for successful `GET`, `PATCH`, or `PUT` calls.
- **`201 Created`**: A new resource was successfully created (typically via `POST`).
  - *Best Practice*: Always include a `Location` header pointing to the newly created resource URI:
    ```http
    HTTP/1.1 201 Created
    Location: /users/105
    ```
- **`202 Accepted`**: The request has been received and queued for asynchronous processing (e.g. video rendering, batch export), but is not yet complete.
- **`204 No Content`**: The request succeeded, and there is no response body to return. Standard for `DELETE` operations.

---

### 🟡 3xx Redirection:
- **`301 Moved Permanently`**: The requested resource has moved permanently to a new URI (specified in `Location` header). Search engines update their indexes.
- **`304 Not Modified`**: Used for **HTTP Caching**. Tells the client: *"The resource has not changed since the timestamp/ETag you supplied in `If-None-Match`. Use your local cached copy!"* (Saves bandwidth).

---

### 🔴 4xx Client Errors (The caller made a mistake):
- **`400 Bad Request`**: Malformed JSON syntax, invalid data types, or missing mandatory fields.
- **`401 Unauthorized`** (Actually means *Unauthenticated*): The caller did not provide authentication credentials (missing/expired JWT or API key).
- **`403 Forbidden`**: The caller is authenticated, but does **not have permission** to access this specific resource (e.g. a regular user trying to access `/admin/metrics`).
  - *Mental Rule*: 401 is *"Who are you?"*; 403 is *"I know who you are, but you cannot enter here."*
- **`404 Not Found`**: The requested URI does not exist. (Can also be used instead of 403 to avoid leaking that a secret resource exists).
- **`405 Method Not Allowed`**: The endpoint exists, but does not support this HTTP verb (e.g. calling `DELETE /login`).
- **`409 Conflict`**: Request cannot be completed due to a conflict with current resource state (e.g. email already registered, or optimistic locking version mismatch).
- **`422 Unprocessable Entity`**: The JSON syntax is valid, but fails business validation rules (e.g. `age: -5` or invalid credit card number format).
- **`429 Too Many Requests`**: The client exceeded their rate limit. Must include `Retry-After: <seconds>` header.

---

### 🔥 5xx Server Errors (The server failed):
- **`500 Internal Server Error`**: Unhandled exception, crash, or fatal bug in server code.
- **`502 Bad Gateway`**: The reverse proxy/load balancer (Nginx/Envoy) received an invalid response or connection drop from the upstream application server.
- **`503 Service Unavailable`**: The server is temporarily overloaded or undergoing maintenance.
- **`504 Gateway Timeout`**: Upstream service (e.g. microservice or database) did not respond in time to the gateway.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [02: Resource Modeling & URIs](02-resource-modeling-and-uris.md) | [REST API Design Hub](README.md) | **[04: Production Checklist](04-what-to-take-care-during-design.md)** |
<!-- /Chapter Navigation Footer -->

