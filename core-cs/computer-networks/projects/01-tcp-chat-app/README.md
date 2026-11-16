# Capstone Project 01: TCP Chat Application with Protocol Framing

---

## 🎯 Overview

One of the most common pitfalls in network programming is treating TCP as a **message-oriented** protocol like UDP. In reality, TCP is a **continuous stream of bytes** with no concept of message boundaries. Packets can be split across multiple `recv()` calls (fragmentation) or merged together into a single `recv()` call (packet coalescing / Nagle's algorithm).

This project implements a robust TCP client-server chat application featuring:
1. **Length-Prefixed Protocol Framing**: Every message is framed with a 4-byte big-endian (`htonl`) payload length header.
2. **Stream Defragmenter**: Accumulates partial reads and extracts discrete, intact application messages.
3. **Automated Verification Harness**: Runs a multi-message fragmentation and coalescing test using connected stream sockets.

---

## 🏗️ Protocol Wire Format

```
+--------------------------+------------------------------------------------+
|  Payload Length (4B)     |              UTF-8 Message Body                |
|  uint32_t Network Order  |               (Length bytes)                   |
+--------------------------+------------------------------------------------+
```

---

## 🚀 Build and Run

```bash
make
make run
```
