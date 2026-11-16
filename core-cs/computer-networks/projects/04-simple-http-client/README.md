# Capstone Project 04: Low-Level Socket HTTP/1.1 Client

---

## 🎯 Overview

Most developers interact with HTTP via high-level libraries (`curl`, `requests`, `fetch`). However, implementing an HTTP client at the raw POSIX socket layer reveals the essential mechanics of network protocols: URL decomposition, socket connection, HTTP/1.1 request formatting, header state machines, and dynamic payload decoding.

### Features
1. **URL Parser**: Deconstructs absolute URLs into scheme, hostname, port, and URI path.
2. **Socket Request Pipeline**: Formats and transmits standard HTTP/1.1 GET/POST requests.
3. **Response Dissector**: Parses response status lines (`HTTP/1.1 200 OK`) and case-insensitive response headers.
4. **RFC 9112 Chunked Transfer Decoder**: Assembles dynamically streamed payloads framed by hex chunk lengths until the terminating `0\r\n\r\n`.
5. **Redirect Follower**: Intelligently handles `301`, `302`, and `307` redirects up to a maximum recursion limit.
6. **Automated Verification Harness**: Runs tests against both `Content-Length` and `Transfer-Encoding: chunked` simulated servers.

---

## 🚀 Build and Run

```bash
make
make run
```
