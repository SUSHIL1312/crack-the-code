# Common Mistakes in the Transport Layer

### 1. Treating TCP Reads as Atomic Message Reads
- **Mistake**: Writing `write(sock, &request, sizeof(request))` on client, and calling `read(sock, &request, sizeof(request))` on server, assuming a single `read()` returns the complete struct.
- **Reality**: TCP is a streaming protocol without message boundaries. A single `read()` may return 10 bytes of a 200-byte struct due to MTU boundaries, network packet fragmentation, or OS buffer chunks.
- **Fix**: Always read in a loop until the full expected length is received:
  ```cpp
  size_t total = 0;
  while (total < sizeof(request)) {
      ssize_t n = read(sock, ((char*)&request) + total, sizeof(request) - total);
      if (n <= 0) break; // Error or EOF
      total += n;
  }
  ```

---

### 2. Assuming UDP Guarantees Message Ordering
- **Mistake**: Sending sequence-dependent commands over UDP (e.g. `Packet 1: CREATE_FILE`, `Packet 2: WRITE_DATA`) without sequencing or ordering logic.
- **Reality**: Intermediate routers forward IP packets independently. Packet 2 can take a faster path through a high-speed fiber link and arrive at the receiver before Packet 1. The application will crash attempting to write to a file that has not been created yet!
- **Fix**: Include an explicit monotonically increasing sequence number in the UDP payload and reorder datagrams in an application jitter buffer.

---

### 3. Exceeding UDP MTU Causing Silent IP Fragmentation Loss
- **Mistake**: Sending an 8 KB payload in a single UDP packet over the public Internet.
- **Reality**: Internet MTU is typically 1,500 bytes. The 8 KB datagram is fragmented into 6 IP fragments. If even **one** of the 6 fragments is dropped by any router along the path, the destination kernel cannot reassemble the datagram and **discards all 6 fragments entirely**! UDP packet loss probability grows exponentially with fragmentation.
- **Fix**: Restrict UDP payloads to safe Path MTU sizes (typically $\le 1,200 - 1,400 \text{ bytes}$) or use TCP / QUIC for large payloads.

---

### 4. Overlooking Checksum Offloading (CSO) in Wireshark
- **Mistake**: Capturing packets locally with Wireshark and seeing "TCP/UDP Checksum Incorrect (0x0000)" warnings, assuming the network card is corrupted.
- **Reality**: Modern NICs perform **Hardware Checksum Offloading (Tx Checksum Offload)**. The OS kernel leaves the checksum field as `0x0000` and delegates the arithmetic to the NIC ASIC on the physical card. Wireshark captures outgoing packets before they reach the NIC, so it sees the uncomputed checksum.
