# Internal Working: Wire Header Bitfield Layouts

The exact binary structures transmitted across physical network wires.

---

## 1. Ethernet II Frame Header Layout (14 Bytes)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Destination MAC Address (Bytes 0-3)          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Destination MAC (Bytes 4-5) |   Source MAC Address (Bytes 0-1) |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Source MAC Address (Bytes 2-5)               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         EtherType (2 Bytes)   |  Payload (IPv4 = 0x0800,      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  ARP = 0x0806, IPv6 = 0x86DD) |
```

- **Destination MAC (6 bytes)**: Physical hardware address of the next-hop node.
- **Source MAC (6 bytes)**: Physical hardware address of the transmitting node.
- **EtherType (2 bytes)**: Indicates the Layer 3 protocol contained in the payload:
  - `0x0800`: IPv4
  - `0x0806`: ARP
  - `0x86DD`: IPv6

---

## 2. IPv4 Header Layout (20 Bytes minimum)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Version|  IHL  |Type of Service|          Total Length         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Identification        |Flags|      Fragment Offset    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Time to Live |    Protocol   |        Header Checksum        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Source IP Address                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Destination IP Address                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- **Version (4 bits)**: Always `4` for IPv4.
- **IHL (Internet Header Length, 4 bits)**: Number of 32-bit words in the header (default 5 words = 20 bytes).
- **Total Length (16 bits)**: Entire packet size in bytes (header + payload, max $65,535$).
- **Time to Live (TTL, 8 bits)**: Decremented by 1 at every router hop. If 0, packet is discarded and `ICMP Time Exceeded` is returned.
- **Protocol (8 bits)**: Next layer protocol:
  - `1`: ICMP
  - `6`: TCP
  - `17`: UDP

---

## 3. TCP Header Layout (20 Bytes minimum)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Source Port          |       Destination Port        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Sequence Number                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Acknowledgment Number                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Data |       |C|E|U|A|P|R|S|F|                               |
| Offset| Rsvd  |W|C|R|C|S|S|Y|I|            Window Size        |
|       |       |R|E|G|K|H|T|N|N|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Checksum            |         Urgent Pointer        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- **Data Offset (4 bits)**: Number of 32-bit words in the TCP header.
- **Flags (9 bits)**:
  - `SYN`: Synchronize sequence numbers (connection initiation).
  - `ACK`: Acknowledgment field valid.
  - `FIN`: Finished transmitting data (graceful close).
  - `RST`: Reset connection immediately (abortive close).
  - `PSH`: Push data directly to application without buffering.
  - `URG`: Urgent pointer field valid.
- **Window Size (16 bits)**: Flow control receive window advertising how many bytes the receiver can accept.
