# Internal Working: ARP Wire Format, Cache States, and 802.1Q VLANs

---

## 1. ARP Packet Wire Format (28 Bytes)

Inside an Ethernet frame with `EtherType = 0x0806`, the ARP payload has the following binary layout:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Hardware Type        |        Protocol Type          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  HW Addr Len  | Proto Addr Len|          Opcode               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Sender Hardware Address (Bytes 0-3)          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Sender HW Addr (Bytes 4-5)    |  Sender Protocol Addr (Bytes 0-1) |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Sender Proto Addr (Bytes 2-3) |  Target Hardware Address (0-1)|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Target Hardware Address (Bytes 2-5)          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Target Protocol Address                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- **Hardware Type (16 bits)**: `1` for Ethernet.
- **Protocol Type (16 bits)**: `0x0800` for IPv4.
- **Hardware Address Length (8 bits)**: `6` (bytes in a MAC).
- **Protocol Address Length (8 bits)**: `4` (bytes in an IPv4).
- **Opcode (16 bits)**:
  - `1`: ARP Request
  - `2`: ARP Reply
  - `3`: RARP Request (Reverse ARP)
  - `4`: RARP Reply
- In an ARP Request, `Target Hardware Address` is set to `00:00:00:00:00:00` because it is unknown.

---

## 2. Linux Kernel ARP Neighbor State Machine

In the Linux kernel, ARP entries are managed by the **Neighbor Table (`struct neighbour`)**:

```
+---------------+     ARP Solicit      +---------------+
|     NONE      | -------------------> |  INCOMPLETE   | (Waiting for ARP Reply)
+---------------+                      +---------------+
                                               |
                                  ARP Reply    v
                                       +---------------+
                                       |  REACHABLE    | (Confirmed active)
                                       +---------------+
                                               |
                                    Timeout    v
                                       +---------------+
                                       |    STALE      | (Needs re-validation)
                                       +---------------+
                                               |
                                  Traffic Sent v
                                       +---------------+
                                       |    DELAY      | (Gives TCP a chance to confirm)
                                       +---------------+
                                               |
                                  Still no ACK v
                                       +---------------+
                                       |    PROBE      | (Sends unicast ARP probes)
                                       +---------------+
                                               |
                                  Probe Fails  v
                                       +---------------+
                                       |    FAILED     | (Dead neighbor)
                                       +---------------+
```

Inspect on Linux via:
```bash
ip neighbor show
# Output: 192.168.1.1 dev eth0 lladdr 52:54:00:12:34:56 REACHABLE
```

---

## 3. IEEE 802.1Q VLAN Tagging (4 Bytes)

When Ethernet frames travel over trunk links connecting switches:
A 4-byte 802.1Q tag is inserted between the Source MAC and EtherType fields:

```
[ Dest MAC (6B) ] [ Src MAC (6B) ] [ 802.1Q Tag (4B) ] [ EtherType (2B) ] [ Payload ]
                                          |
        +---------------------------------+---------------------------------+
        | TPID (16 bits) = 0x8100 | PCP (3 bits) | DEI (1 bit) | VID (12 bits)  |
        +-------------------------------------------------------------------+
```
- **TPID (Tag Protocol Identifier)**: Fixed value `0x8100`.
- **PCP (Priority Code Point)**: 3-bit Class of Service (QoS).
- **VID (VLAN Identifier, 12 bits)**: Supports $2^{12} = 4,096$ isolated virtual LANs on the same physical switch hardware!
