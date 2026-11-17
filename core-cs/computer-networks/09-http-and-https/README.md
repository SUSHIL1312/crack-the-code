# Module 09: HTTP, HTTPS, and Web Protocols

<!-- Chapter Navigation Header -->
| ⬅️ [Module 08: DNS](../08-dns/README.md) | 🏠 [Computer Networks Hub](../README.md) | [Module 10: TLS & Security](../10-tls-and-security/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## Overview
Hypertext Transfer Protocol (HTTP) is the foundational application layer protocol of the World Wide Web and the universal communication medium for RESTful APIs, microservices, and mobile applications. Understanding its architectural evolution from simple ASCII text (HTTP/1.1) to binary multiplexed streams (HTTP/2) and UDP-based QUIC framing (HTTP/3) is vital for high-scale backend engineering and FAANG architecture interviews.

In this module, you will master the HTTP request/response model, caching semantics (`ETag`, `Cache-Control`), REST principles (safety, idempotency), HTTP/2 binary framing and HPACK, and implement a robust HTTP/1.1 parser in C++.

---

## Learning Objectives
1. **HTTP Protocol Evolution**:
   - HTTP/1.0 vs HTTP/1.1 (Persistent Keep-Alive, Chunked Transfer Encoding).
   - HTTP/2 (Binary Framing Layer, Stream Multiplexing, HPACK Header Compression).
   - HTTP/3 (HTTP semantics mapped onto QUIC streams, QPACK).
2. **RESTful Architecture**:
   - Safe vs Idempotent operations (`GET`, `POST`, `PUT`, `PATCH`, `DELETE`).
   - Standard HTTP Status Codes (200, 201, 204, 301, 304, 400, 401, 403, 404, 409, 429, 500, 502, 503, 504).
3. **HTTP Caching Mechanics**: Master conditional requests with `ETag` / `If-None-Match`, `Last-Modified` / `If-Modified-Since`, and cache validation (`304 Not Modified`).
4. **Chunked Transfer Encoding**: Stream dynamic, unknown-length payloads with `Transfer-Encoding: chunked`.
5. **Hands-On Parsing**: Build and verify an RFC-compliant HTTP parser that extracts methods, paths, headers, and chunked bodies.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of HTTP protocols, framing, REST semantics, and caching headers.
- `internal-working.md`: Wire formats of HTTP/1.1 vs HTTP/2 binary frames (DATA, HEADERS, SETTINGS) and HPACK compression tables.
- `visual-explanation.md`: Visual diagrams comparing HTTP/1.1 Keep-Alive, HTTP/2 Stream Multiplexing, and Chunked Transfer Encoding.
- `examples/`:
  - `http_parser_and_framing.cpp`: Complete C++ HTTP parser with chunked body decoding and header dictionary management.
- `exercises/`: Protocol analysis and API design challenges.
- `solutions/`: Reference solutions.
- `interview-questions.md`: FAANG interview questions (e.g. `PUT` vs `PATCH`, 502 Bad Gateway vs 504 Gateway Timeout, ETag validation).
- `common-mistakes.md`: Breaking idempotency in `GET` requests, missing `Connection: keep-alive` in client pools, parsing HTTP with brittle regexes.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 08: DNS](../08-dns/README.md) | [Computer Networks Hub](../README.md) | **[Module 10: TLS & Security](../10-tls-and-security/README.md)** |
<!-- /Chapter Navigation Footer -->

