# 05: End-to-End Design Case Study: Enterprise E-Commerce API

<!-- Chapter Navigation Header -->
| ⬅️ [04: Production Checklist](04-what-to-take-care-during-design.md) | 🏠 [REST API Design Hub](README.md) | [06: FAANG Interview Questions](interview-questions.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

In this case study, we will design the complete REST API for an enterprise E-Commerce platform (like Amazon or Shopify). We will apply all the architectural rules, status codes, idempotency keys, and error handling discussed in previous chapters.

---

## 1. Step 1: Identify Resources and Relationships

First, map out the domain nouns:
- **`Product`**: Items available for purchase.
- **`Cart`**: Temporary shopping container for a customer.
- **`Order`**: Committed purchase agreement.
- **`Payment`**: Monetary transaction associated with an order.

```
Customer ──► Cart ──(Checkout)──► Order ──► Payment
              │                     │
              ▼                     ▼
           Products            Order Items
```

---

## 2. Step 2: Define the Resource Endpoints Table

| HTTP Verb | Endpoint | Purpose | Idempotent? | Status Code |
|---|---|---|---|---|
| `GET` | `/v1/products` | Browse catalog with cursor pagination & filters | **Yes** | `200 OK` |
| `GET` | `/v1/products/{id}` | Get product details | **Yes** | `200 OK` |
| `GET` | `/v1/cart` | View current user's active cart | **Yes** | `200 OK` |
| `POST`| `/v1/cart/items` | Add product to cart | **No** | `201 Created` |
| `DELETE`| `/v1/cart/items/{itemId}` | Remove item from cart | **Yes** | `204 No Content` |
| `POST`| `/v1/orders` | Place order (Checkout) with `Idempotency-Key` | **Yes (via key)** | `201 Created` |
| `GET` | `/v1/orders/{id}` | Track order status | **Yes** | `200 OK` |
| `POST`| `/v1/orders/{id}/cancel` | Cancel order before shipment | **Yes** | `200 OK` |
| `POST`| `/v1/orders/{id}/payments`| Initiate payment transaction | **Yes (via key)** | `201 Created` |

---

## 3. Step 3: Detailed Request & Response Contracts

### Endpoint 1: Browse Products (Cursor Pagination + Filtering)
```http
GET /v1/products?category=electronics&limit=2&cursor=cHJkXzAwMg== HTTP/1.1
Host: api.store.com
Authorization: Bearer <jwt_token>
```

#### Response:
```http
HTTP/1.1 200 OK
Content-Type: application/json
Cache-Control: public, max-age=300
ETag: "w/prod-hash-789"

{
  "data": [
    {
      "id": "prd_003",
      "title": "Wireless Noise-Cancelling Headphones",
      "price_cents": 29900,
      "currency": "usd",
      "stock": 45,
      "category": "electronics"
    },
    {
      "id": "prd_004",
      "title": "Mechanical Gaming Keyboard",
      "price_cents": 12900,
      "currency": "usd",
      "stock": 18,
      "category": "electronics"
    }
  ],
  "pagination": {
    "limit": 2,
    "has_more": true,
    "next_cursor": "cHJkXzAwNA=="
  }
}
```

---

### Endpoint 2: Place Order (With Idempotency Key)
To prevent accidental double purchases when a customer taps "Checkout", the client supplies an `Idempotency-Key`.

```http
POST /v1/orders HTTP/1.1
Host: api.store.com
Authorization: Bearer <jwt_token>
Idempotency-Key: a1b2c3d4-e5f6-7890-abcd-ef1234567890
Content-Type: application/json

{
  "shipping_address": {
    "street": "100 Market St",
    "city": "San Francisco",
    "state": "CA",
    "postal_code": "94105",
    "country": "US"
  },
  "items": [
    {
      "product_id": "prd_003",
      "quantity": 1
    }
  ]
}
```

#### Response:
```http
HTTP/1.1 201 Created
Location: /v1/orders/ord_88291
Content-Type: application/json
ETag: "ord-v1-hash"

{
  "id": "ord_88291",
  "status": "pending_payment",
  "customer_id": "cus_1048",
  "currency": "usd",
  "subtotal_cents": 29900,
  "tax_cents": 2691,
  "total_cents": 32591,
  "created_at": "2026-11-17T02:30:00Z",
  "_links": {
    "self": { "href": "/v1/orders/ord_88291" },
    "payment": { "href": "/v1/orders/ord_88291/payments" },
    "cancel": { "href": "/v1/orders/ord_88291/cancel" }
  }
}
```

---

### Endpoint 3: Process Payment (Safe Financial Transaction)
```http
POST /v1/orders/ord_88291/payments HTTP/1.1
Host: api.store.com
Authorization: Bearer <jwt_token>
Idempotency-Key: pay_unique_998124_key
Content-Type: application/json

{
  "payment_method_id": "pm_card_visa_4242",
  "amount_cents": 32591,
  "currency": "usd"
}
```

#### Response (Success):
```http
HTTP/1.1 201 Created
Location: /v1/orders/ord_88291/payments/pay_77182
Content-Type: application/json

{
  "id": "pay_77182",
  "order_id": "ord_88291",
  "status": "succeeded",
  "amount_cents": 32591,
  "currency": "usd",
  "gateway_transaction_id": "ch_3N8zYx2eZvKYlo2C0",
  "created_at": "2026-11-17T02:30:05Z"
}
```

---

### Endpoint 4: Error Handling Scenario (Insufficient Inventory)
Suppose another buyer purchased the last pair of headphones 100 milliseconds earlier.

```http
HTTP/1.1 409 Conflict
Content-Type: application/problem+json

{
  "type": "https://api.store.com/errors/insufficient-stock",
  "title": "Inventory Conflict",
  "status": 409,
  "detail": "Requested quantity (1) exceeds available stock (0) for product 'prd_003'.",
  "instance": "/v1/orders",
  "product_id": "prd_003",
  "available_stock": 0,
  "trace_id": "trace_990182_abc"
}
```

---

## 4. Key Takeaways for System Design Interviews
1. **Always use type-prefixed IDs** (`prd_...`, `ord_...`, `pay_...`) to make APIs self-documenting and prevent integer ID crawling.
2. **Represent amounts in the smallest currency unit** (`price_cents: 29900` instead of `299.00`) to completely eliminate floating-point rounding errors.
3. **Embed hypermedia links (`_links`)** for intuitive state transitions (`cancel`, `payment`).
4. **Mandate `Idempotency-Key` on payment/order POST requests** to make distributed systems fault-tolerant against network disconnects.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [04: Production Checklist](04-what-to-take-care-during-design.md) | [REST API Design Hub](README.md) | **[06: FAANG Interview Questions](interview-questions.md)** |
<!-- /Chapter Navigation Footer -->

