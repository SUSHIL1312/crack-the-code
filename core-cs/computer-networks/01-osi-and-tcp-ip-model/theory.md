# Theoretical Foundations: OSI and TCP/IP Layer Models

---

## 1. The 7-Layer OSI Reference Model

Standardized by ISO (ISO/IEC 7498-1), the OSI model divides communication into seven logical tiers:

| Layer # | Layer Name | Protocol Data Unit (PDU) | Primary Responsibility | Common Protocols |
| :--- | :--- | :--- | :--- | :--- |
| **7** | **Application** | Data / Message | Human-computer interaction, application-level network services. | HTTP, DNS, SMTP, SSH, FTP |
| **6** | **Presentation** | Data | Data format translation, character encoding (ASCII/UTF-8), encryption/decryption, compression. | TLS/SSL, MIME, gzip, JSON/Protobuf |
| **5** | **Session** | Data | Establishing, managing, and terminating dialogue sessions between applications. | RPC, NetBIOS, SIP |
| **4** | **Transport** | Segment (TCP) / Datagram (UDP) | End-to-end process-to-process delivery, flow control, error recovery, multiplexing via port numbers. | TCP, UDP, SCTP, QUIC |
| **3** | **Network** | Packet | Host-to-host routing across disparate networks, logical addressing. | IPv4, IPv6, ICMP, BGP, OSPF |
| **2** | **Data Link** | Frame | Node-to-node frame transfer across a single physical link, physical MAC addressing, media access control. | Ethernet (802.3), Wi-Fi (802.11), PPP |
| **1** | **Physical** | Bits | Physical transmission of raw binary electrical, optical, or radio frequency pulses across cables/air. | 1000BASE-T, Fiber optics, DSL, Radio waves |

---

## 2. The Pragma of TCP/IP (4/5 Layers)

The TCP/IP model was designed pragmatically by DARPA:
- **Application Layer**: Merges OSI Layers 7, 6, and 5 into a single user-space application layer.
- **Transport Layer**: Identical to OSI Layer 4 (`TCP`, `UDP`).
- **Internet Layer**: Identical to OSI Layer 3 (`IP`, `ICMP`).
- **Network Access / Link Layer**: Combines OSI Layers 2 and 1 (or separated as Data Link and Physical).

```
   OSI 7-LAYER MODEL                       TCP/IP 5-LAYER MODEL
+-----------------------+              +-----------------------+
| 7. Application        |              |                       |
+-----------------------+              | 5. Application        |
| 6. Presentation       | ---------->  |    (HTTP, TLS, DNS)   |
+-----------------------+              |                       |
| 5. Session            |              +-----------------------+
+-----------------------+              | 4. Transport          |
| 4. Transport          | ---------->  |    (TCP, UDP, QUIC)   |
+-----------------------+              +-----------------------+
| 3. Network            | ---------->  | 3. Network / Internet |
|                       |              |    (IPv4, IPv6, ICMP) |
+-----------------------+              +-----------------------+
| 2. Data Link          | ---------->  | 2. Data Link          |
|                       |              |    (Ethernet, 802.11) |
+-----------------------+              +-----------------------+
| 1. Physical           | ---------->  | 1. Physical           |
|                       |              |    (Fiber, Copper)    |
+-----------------------+              +-----------------------+
```

---

## 3. Encapsulation & Decapsulation

When an application sends data:
1. **Application Layer**: Generates application payload $M$ (e.g. `GET /index.html HTTP/1.1`).
2. **Transport Layer**: Prepends a Transport Header $H_t$ (e.g. TCP 20-byte header containing source port, destination port, sequence number) $\implies$ **TCP Segment**.
3. **Network Layer**: Prepends an IP Header $H_n$ (e.g. IPv4 20-byte header containing Source IP, Destination IP, TTL) $\implies$ **IP Packet**.
4. **Data Link Layer**: Prepends a Data Link Header $H_d$ (Ethernet 14-byte header containing Source MAC, Dest MAC, EtherType) and appends a Frame Check Sequence trailer $T_d$ (4-byte CRC32) $\implies$ **Ethernet Frame**.
5. **Physical Layer**: Converts the frame into physical analog/digital pulses transmitted onto the wire.

Upon reception, each intermediate router examines and strips up to Layer 3 ($H_d$ and $H_n$), recalculates the checksum, updates TTL, prepends a new Layer 2 header for the next hop link, and transmits. The final destination decapsulates all layers back up to Layer 7.
