# 01: REST Fundamentals Explained Simply

## What is REST?
**REST** stands for **Representational State Transfer**. It was introduced in 2000 by Roy Fielding in his doctoral dissertation to describe how the World Wide Web successfully scaled to billions of users and servers.

### The Plain-English Explanation:
Imagine you walk into a library:
- Every book on the shelf has a unique call number (URI).
- You can read a book (`GET`), donate a new book (`POST`), replace a damaged book with a new copy (`PUT`), fix a torn page (`PATCH`), or discard a book (`DELETE`).
- The librarian doesn't need to remember who you were yesterday to hand you a book today. Every request you make contains all the information needed to fulfill it (**Stateless**).
- You don't receive the physical printing press; you receive a **representation** of the book (printed text, PDF, or audio format).

That is REST in a nutshell!

---

## The 6 Architectural Constraints of REST

To call an API "RESTful", it must adhere to 6 architectural constraints:

### 1. Client-Server Separation
* **Analogy**: A restaurant dining room and kitchen.
* **Concept**: The client (mobile app, browser, frontend) handles the user interface and presentation. The server handles business logic and database persistence.
* **Benefit**: The frontend can be rewritten from React to Swift without changing a single line of backend server code.

### 2. Statelessness (Most Important!)
* **Analogy**: Mailing letters via the postal service. Each letter must contain the full sender and recipient address. The mail carrier does not remember who you wrote to last Tuesday.
* **Concept**: The server does NOT store any client session context between requests. Every single HTTP request from the client must contain all authentication credentials (e.g., Bearer tokens) and parameters necessary to fulfill that request.
* **Benefit**: Massive scalability! If you have 50 server instances behind a load balancer, any server can process any incoming request without sharing in-memory session states.

### 3. Cacheability
* **Analogy**: Taking a photo of a restaurant's weekly menu on your phone so you don't have to walk back to the restaurant just to check opening hours.
* **Concept**: Every server response must explicitly declare whether it can be cached by the client or intermediate proxies (`Cache-Control: public, max-age=3600`, `ETag`).
* **Benefit**: Dramatically reduces network latency and server load.

### 4. Layered System
* **Analogy**: Ordering a package on Amazon. You place the order with Amazon; Amazon routes it through logistics centers, cargo planes, and local couriers. You don't know or care how many intermediate layers handled your package.
* **Concept**: The client cannot tell whether it is connected directly to the end server or an intermediate proxy, load balancer, API gateway, or CDN.
* **Benefit**: Enables transparent caching (Cloudflare), security firewalls (WAF), and load balancing without altering client code.

### 5. Uniform Interface
* **Analogy**: Standard three-prong electrical plugs. Every device (laptop charger, toaster, television) connects to the wall using the exact same standard interface.
* **Concept**: All resources are accessed and manipulated using standard HTTP conventions (URIs, HTTP methods, headers, status codes) rather than proprietary custom protocols.
* **Core sub-principles**:
  - Identification of resources via URIs (`/users/123`).
  - Manipulation of resources through representations (JSON/XML).
  - Self-descriptive messages (Headers like `Content-Type: application/json`).
  - Hypermedia as the Engine of Application State (HATEOAS).

### 6. Code on Demand (Optional)
* **Concept**: Servers can temporarily extend client functionality by transferring executable code (e.g., JavaScript scripts or WebAssembly binaries).
* **Note**: This is the only optional constraint in REST.

---

## Why REST Matters in System Design Interviews
In FAANG interviews, interviewers evaluate whether you understand **why** REST is chosen over alternatives like WebSockets, gRPC, or GraphQL:
- **Choose REST** when building public-facing APIs, web/mobile backends where caching, wide client compatibility, and firewall traversal are paramount.
- **Choose gRPC** for internal low-latency microservice-to-microservice communication (Protobuf binary serialization, HTTP/2 multiplexing).
- **Choose WebSockets** for bidirectional real-time event streaming (chat apps, stock tickers, gaming).
- **Choose GraphQL** when clients have complex, deeply nested, dynamically varying data requirements and want to prevent over-fetching.
