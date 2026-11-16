# Visual Explanation: HTTP Evolution and Chunked Streaming

---

## 1. HTTP/1.1 Pipelining vs HTTP/2 Multiplexing

```
HTTP/1.1 SEQUENTIAL (Connection: keep-alive)
Request 1 ====>
             <==== Response 1
Request 2 ========>
                 <======== Response 2
(Must wait for Response 1 before sending Request 2!)


HTTP/1.1 PIPELINING (Failed due to HoL Blocking)
Request 1 ====>
Request 2 ========>
             <==== Response 1
                 <======== Response 2
(If Request 1 is a slow database query, Response 2 is BLOCKED on the server,
even if Response 2 was ready in 1 millisecond!)


HTTP/2 MULTIPLEXING (Interleaved Binary Frames)
[ Stream 1 HEADERS ] ====>
[ Stream 3 HEADERS ] ========>
[ Stream 1 DATA    ] ====>
             <==== [ Stream 3 DATA (Fast Response 2 arrives FIRST!) ]
             <==== [ Stream 1 DATA (Slow Response 1 finishes later) ]
(Zero application-layer blocking; server responds as data becomes ready!)
```

---

## 2. Chunked Transfer Encoding Format (`Transfer-Encoding: chunked`)

When an HTTP/1.1 server generates a response dynamically (e.g. streaming LLM tokens, large file export) and **does not know the total Content-Length in advance**:

```
HTTP/1.1 200 OK\r\n
Content-Type: text/plain\r\n
Transfer-Encoding: chunked\r\n
\r\n
7\r\n               <-- Chunk 1 length in hex (7 bytes)
Mozilla\r\n         <-- Chunk 1 data
9\r\n               <-- Chunk 2 length in hex (9 bytes)
Developer\r\n       <-- Chunk 2 data
7\r\n               <-- Chunk 3 length in hex (7 bytes)
Network\r\n         <-- Chunk 3 data
0\r\n               <-- Final chunk length (0 indicates End of Stream!)
\r\n                <-- Terminating empty line
```
The client knows the transmission is complete upon receiving the terminal `0\r\n\r\n` chunk, without needing the server to close the underlying TCP connection!
