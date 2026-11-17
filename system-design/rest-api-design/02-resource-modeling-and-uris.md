# 02: Resource Modeling and URI Design

<!-- Chapter Navigation Header -->
| ⬅️ [01: REST Fundamentals](01-rest-fundamentals.md) | 🏠 [REST API Design Hub](README.md) | [03: HTTP Verbs & Status Codes](03-http-verbs-and-status-codes.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. The Core Rule: Resources are Nouns, Not Verbs
The most common mistake junior engineers make in API design is putting actions (verbs) into the URL:

```
❌ WRONG (RPC-style URL anti-pattern):
POST /createUser
GET  /getUserById?id=42
POST /updateUserAddress?id=42
POST /deleteUser?id=42

✅ CORRECT (RESTful Resource-Oriented Modeling):
POST   /users            (Create a new user)
GET    /users/42         (Fetch user 42)
PATCH  /users/42         (Update user 42's address)
DELETE /users/42         (Delete user 42)
```
**Why?** In REST, the **URI identifies the Resource** (the noun), while the **HTTP Method defines the Action** (the verb).

---

## 2. Plural Nouns vs Singular Nouns
Always use **plural nouns** for collection resources:
- Collection resource: `/users`, `/products`, `/orders`
- Individual resource within collection: `/users/{userId}`, `/products/{productId}`

### Why Plural?
Consider querying a list vs querying an item:
- `GET /users` -> Returns a collection (array) of users.
- `GET /users/42` -> Returns a single user object.
If you used singular `/user`, `GET /user` reading a list reads unnaturally. Consistency across all endpoints reduces API cognitive load.

---

## 3. Hierarchical and Nested Resources
Real-world entities have relationships. REST models relationships using nested URI paths:

### Parent-Child Relationships:
- A user has orders:
  - `GET /users/101/orders` -> List all orders placed by user 101.
  - `POST /users/101/orders` -> Create a new order for user 101.
  - `GET /users/101/orders/5001` -> Fetch order 5001 specifically for user 101.

### ⚠️ The Two-Level Depth Rule:
Never nest resources more than **2 levels deep**! Deeply nested URLs become unwieldy, fragile, and hard to cache.

```
❌ AVOID Deep Nesting:
GET /departments/5/teams/12/projects/99/tasks/101/comments

✅ PREFER Flatter Endpoints with Query Filters:
GET /tasks/101/comments
GET /tasks?project_id=99&team_id=12
```
If a resource has a globally unique ID (like a UUID or database primary key), expose it at top-level: `/tasks/101` rather than forcing the client to supply every ancestor ID!

---

## 4. Path Parameters vs Query Parameters: When to Use Which?

| Parameter Type | Syntax Example | When to Use |
|---|---|---|
| **Path Parameter** | `/orders/{orderId}` | Identifying a **specific resource** or direct parent hierarchy. Mandatory for routing. |
| **Query Parameter** | `/orders?status=shipped&limit=20` | **Filtering, Sorting, Searching, and Pagination**. Optional parameters that refine the result set without changing what resource is being accessed. |

### Practical Examples:
```http
# Fetch user with ID 42 (Path Parameter)
GET /users/42

# Filter users who are active, sorted by creation date (Query Parameters)
GET /users?status=active&sort=-created_at

# Full-text search for books matching "distributed systems" (Query Parameter)
GET /books?q=distributed+systems

# Paginated results (Query Parameters)
GET /products?limit=25&cursor=eyJpZCI6MTAwfQ==
```

---

## 5. What About Non-CRUD Actions? (Handling Complex Operations)
In real applications, not every business action fits neatly into Create, Read, Update, Delete.
*How do you model operations like "cancel order", "send verification email", or "checkout cart"?*

There are 3 standard industry patterns:

### Pattern A: Treat the Action as a Sub-Resource Controller
Append a verb or controller action as a sub-resource with `POST`:
```http
POST /orders/42/cancel
POST /users/101/send-verification-email
POST /checkout
```

### Pattern B: Treat the State Transition as a Resource
Model the transition as an independent resource:
```http
# Instead of POST /orders/42/cancel:
POST /orders/42/cancellations
```

### Pattern C: Update State via PATCH
If the action is simply toggling a state field:
```http
PATCH /orders/42
Content-Type: application/json

{
  "status": "cancelled",
  "cancellation_reason": "Customer requested refund"
}
```
In FAANG interviews, **Pattern A** and **Pattern C** are widely accepted and used by Stripe, GitHub, and Google Cloud APIs.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [01: REST Fundamentals](01-rest-fundamentals.md) | [REST API Design Hub](README.md) | **[03: HTTP Verbs & Status Codes](03-http-verbs-and-status-codes.md)** |
<!-- /Chapter Navigation Footer -->

