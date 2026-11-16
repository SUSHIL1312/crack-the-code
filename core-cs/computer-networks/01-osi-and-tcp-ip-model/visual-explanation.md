# Visual Explanation: Encapsulation Across Network Devices

---

## 1. The Onion Model of Packet Encapsulation

```
Application Data: [ "GET /api/v1/user HTTP/1.1" ]
                               |
                               v
Transport Segment:
+----------------+-------------------------------------+
| TCP Header     | Application Data                    |
| (20 bytes)     | "GET /api/v1/user HTTP/1.1"         |
+----------------+-------------------------------------+
                               |
                               v
Network Packet:
+---------------+----------------+-------------------------------------+
| IPv4 Header   | TCP Header     | Application Data                    |
| (20 bytes)    | (20 bytes)     | "GET /api/v1/user HTTP/1.1"         |
+---------------+----------------+-------------------------------------+
                               |
                               v
Data Link Frame:
+-----------------+---------------+----------------+-----------------------+----------+
| Ethernet Header | IPv4 Header   | TCP Header     | Application Data      | CRC32    |
| (14 bytes)      | (20 bytes)    | (20 bytes)     |                       | (4 bytes)|
+-----------------+---------------+----------------+-----------------------+----------+
```

---

## 2. Who Looks at What Layer in the Path?

```
[ Sender Host ]              [ L2 Switch ]                 [ L3 Router ]               [ Receiver Host ]
+-------------+             +-------------+               +-------------+             +-------------+
| Layer 7: App|             |             |               |             |             | Layer 7: App|
| Layer 4: TCP|             |             |               |             |             | Layer 4: TCP|
| Layer 3: IP |             |             |               | Layer 3: IP |             | Layer 3: IP |
| Layer 2: MAC| ----------> | Layer 2: MAC| ------------> | Layer 2: MAC| ----------> | Layer 2: MAC|
| Layer 1: PHY|             | Layer 1: PHY|               | Layer 1: PHY|             | Layer 1: PHY|
+-------------+             +-------------+               +-------------+             +-------------+

Sender inspects:             Switch inspects:              Router inspects:            Receiver inspects:
All Layers 1-7               Only Layer 2 (MAC)            Layers 2 & 3 (MAC & IP)     All Layers 1-7
```

Notice that an intermediate L2 Switch does not look inside the IP or TCP packet at all; it forwards solely based on the 48-bit destination MAC address.
An intermediate L3 Router inspects the destination IP address, decrements the TTL, and swaps the Layer 2 Ethernet header.
