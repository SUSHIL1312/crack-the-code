# Common Mistakes in OSI & TCP/IP Layer Models

### 1. Forgetting Network Byte Order (`htons` / `ntohs`)
- **Mistake**: Writing integer port numbers or lengths directly into raw packet structs on x86/ARM machines without conversion (e.g., `hdr.dst_port = 80;`).
- **Reality**: The Internet wire format specifies **Big-Endian** (most significant byte first) as Network Byte Order. Most consumer and server CPUs (x86_64, Apple Silicon ARM64) use **Little-Endian**.
  - Writing `80` (`0x0050`) directly on Little-Endian sets the wire bytes to `0x50 0x00`, which is port `20480` on the network!
- **Fix**: Always wrap multi-byte integers in `htons()` (Host to Network Short), `htonl()` (Host to Network Long), `ntohs()`, and `ntohl()`.

---

### 2. Assuming Routers Reassemble IP Fragments
- **Mistake**: Believing that if Router 1 fragments a 4,000-byte IP packet into three fragments, Router 2 will reassemble them into one packet before forwarding.
- **Reality**: Intermediate routers **never** reassemble fragments. Reassembly is strictly an end-to-end operation performed by the final destination host's kernel IP stack. Reassembling at routers would exhaust router memory and crash high-speed forwarding pipelines.

---

### 3. Believing Layer 2 (MAC) Addresses Persist Across the Internet
- **Mistake**: Believing that when a browser in New York connects to `google.com` in California, the server at Google sees the client's laptop MAC address.
- **Reality**: MAC addresses are local to a single physical link / broadcast domain. At every single router hop, the source MAC is rewritten to the router's egress MAC, and the destination MAC is rewritten to the next-hop router's MAC. Only IP addresses survive hop-to-hop across the Internet.

---

### 4. Overlooking Header Padding & Alignment in Custom Structs
- **Mistake**: Defining C++ structs for protocol headers without `#pragma pack(push, 1)` or `__attribute__((packed))`.
- **Reality**: Modern C++ compilers insert padding bytes into structs to align 16-bit and 32-bit fields to 4-byte or 8-byte boundaries. If padding is added to an Ethernet or IP struct, the struct size will not match the wire format, leading to offset corruption and invalid packet parsing.
