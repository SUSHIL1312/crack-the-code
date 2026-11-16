# Capstone Project 03: HTTP/1.1 Server From Scratch

---

## 🎯 Overview

HTTP is the foundational application layer protocol powering the World Wide Web and cloud microservices. Rather than relying on external web frameworks, this project constructs a compliant HTTP/1.1 server from raw POSIX socket primitives.

### Features
1. **RFC 9112 Parser**: Parses request lines (`METHOD /path HTTP/1.1`), case-insensitive HTTP headers, and message bodies.
2. **REST-Style Routing Engine**: Routes requests based on HTTP method and path (`GET /`, `GET /api/status`, `POST /api/echo`).
3. **Response Serialization**: Generates RFC-compliant HTTP status lines, MIME `Content-Type` headers, and accurate `Content-Length`.
4. **Error Handling**: Graceful responses for `404 Not Found`, `400 Bad Request`, and `405 Method Not Allowed`.
5. **Automated Verification Harness**: Sends multiple HTTP/1.1 queries and verifies status codes, headers, and body payloads.

---

## 🚀 Build and Run

```bash
make
make run
```
