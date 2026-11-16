# Internal Working: HTTP/2 Binary Frames & HPACK Compression

---

## 1. HTTP/2 Binary Framing Layer

Unlike HTTP/1.1 which formats messages as human-readable ASCII lines separated by CRLF (`\r\n`), HTTP/2 divides all communication into binary-encoded **Frames**:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                 Length (24 bits)              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type (8b)   |   Flags (8b)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|R|                 Stream Identifier (31 bits)                 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Frame Payload (0..Length)                 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- **Length (24 bits)**: Size of the frame payload (default max $2^{14} = 16,384 \text{ bytes}$).
- **Type (8 bits)**:
  - `0x00 DATA`: Transmits application body bytes.
  - `0x01 HEADERS`: Transmits compressed HTTP headers and opens a new stream.
  - `0x02 PRIORITY`: Specifies stream dependency and weight.
  - `0x03 RST_STREAM`: Immediately cancels a stream without tearing down the TCP connection.
  - `0x04 SETTINGS`: Configuration parameters (max frame size, flow control window).
  - `0x06 PING`: Heartbeat / RTT measurement.
  - `0x07 GOAWAY`: Graceful shutdown signal informing peer of the last processed stream ID.
- **Flags (8 bits)**: e.g. `END_STREAM (0x01)` signals this is the last frame for this stream.
- **Stream Identifier (31 bits)**: Client-initiated streams use **odd numbers** (1, 3, 5...); server-initiated streams (Server Push) use **even numbers** (2, 4, 6...); stream 0 is reserved for connection-level control frames.

---

## 2. HPACK Header Compression (RFC 7541)

In HTTP/1.1, clients repeatedly transmit identical headers (e.g., `User-Agent: Mozilla/5.0...`, `Accept-Language`, `Cookie: session_id=...`) with every single asset request, wasting 500 to 2,000 bytes per request.
Traditional gzip compression could not be used due to security vulnerabilities (CRIME and BREACH attacks).

HPACK solves this using two coordinated tables:
1. **Static Table**: A hardcoded 61-entry table of ubiquitous headers:
   - Index 2: `method: GET`
   - Index 3: `method: POST`
   - Index 6: `scheme: http`
   - Index 7: `scheme: https`
   - Index 14: `status: 200`
   - To send `:method: GET`, HPACK transmits only **1 single byte (`0x82`)**!
2. **Dynamic Table**: A FIFO sliding window table populated as headers arrive.
   - When the client sends `authorization: Bearer eyJhbGci...` for the first time, it is stored in the dynamic table at Index 62.
   - Subsequent requests over the connection represent that entire 500-byte token with a **2-byte integer index (`0xBE`)**, reducing header overhead by over 85%!
