# Common Mistakes in HTTP & Web Protocols

### 1. Causing Side Effects in `GET` Requests
- **Mistake**: Implementing endpoints like `GET /api/v1/delete_user?id=42` or `GET /logout`.
- **Reality**: RFC 9110 specifies that `GET` methods **must be safe (read-only)**. Web crawlers (Googlebot, Bingbot), browser link prefetchers, and CDN caches proactively fetch `GET` URLs in the background. If a `GET` URL deletes data or logs users out, search engine spiders will silently wipe customer accounts or log everyone out!
- **Fix**: Use `POST`, `DELETE`, or `PUT` for any operation that alters state.

---

### 2. Disabling HTTP Keep-Alive in Microservice Client Pools
- **Mistake**: Instantiating a new HTTP client per API request in backend services without connection pooling.
- **Reality**: Creating a new TCP + TLS connection per microservice call adds 20-50 ms of handshake latency to every RPC, exhausts local ephemeral ports into `TIME_WAIT`, and places heavy CPU load on TLS key negotiation.
- **Fix**: Use a shared, persistent connection pool (e.g. Apache HttpClient, Go `http.Client` with `Transport.MaxIdleConns`, Python `requests.Session`).

---

### 3. Parsing HTTP Headers with Fragile Regular Expressions
- **Mistake**: Using simple regexes like `/Content-Length: (.*)/` to parse incoming HTTP headers.
- **Reality**: HTTP/1.1 header syntax allows:
  - Whitespace variation: `Content-Length:100` or `Content-Length:   100  `.
  - Case insensitivity: `content-length`, `Content-Length`, `CONTENT-LENGTH`.
  - Obsolete line folding (obs-fold).
  - Malformed regexes make servers vulnerable to **HTTP Request Smuggling** (CVE-2020-12440) where attackers desynchronize front-end proxies and back-end servers!
- **Fix**: Use battle-tested, state-machine HTTP parsers (e.g. `llhttp`, `nghttp2`).

---

### 4. Confusing `301 Moved Permanently` with `302 Found`
- **Mistake**: Using `301` for temporary maintenance redirects or A/B testing splits.
- **Reality**: Browsers aggressively cache `301 Moved Permanently` responses in their local disk cache indefinitely (often bypassing TTL). Even after you revert the server configuration, existing users' browsers will continue redirecting to the old destination until they manually clear their browser cache!
- **Fix**: Use `302 Found` or `307 Temporary Redirect` for any redirect that might change in the future.
