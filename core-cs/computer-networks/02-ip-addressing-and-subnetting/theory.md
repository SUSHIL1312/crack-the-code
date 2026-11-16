# Theoretical Foundations: IP Addressing and Subnetting

---

## 1. IPv4 Address Architecture

An IPv4 address is an unsigned 32-bit binary integer, conventionally expressed in **dotted-decimal notation** where each byte (octet) ranges from 0 to 255:

$$\text{192.168.1.1} \iff \texttt{11000000.10101000.00000001.00000001}_2 \iff \text{0xC0A80101}_{16}$$

Every IPv4 address consists of two components:
1. **Network Prefix**: Identifies the specific network or subnetwork.
2. **Host Identifier**: Identifies the specific host interface on that subnetwork.

---

## 2. Historical Classful Addressing vs Modern CIDR

### The Historical Classes (Obsolete since 1993)
- **Class A (`0...`)**: `/8` mask (`255.0.0.0`). 126 networks, 16.7 million hosts each.
- **Class B (`10...`)**: `/16` mask (`255.255.0.0`). 16,384 networks, 65,534 hosts each.
- **Class C (`110...`)**: `/24` mask (`255.255.255.0`). 2 million networks, 254 hosts each.
- **Class D (`1110...`)**: Multicast (`224.0.0.0` to `239.255.255.255`).
- **Class E (`1111...`)**: Experimental / Reserved.

*Why Classful Died*: Massive address waste. An organization needing 300 hosts was forced to receive a Class B network (65,534 addresses), wasting 65,234 IP addresses!

### CIDR: Classless Inter-Domain Routing (RFC 1519)
CIDR completely eliminated address classes. A network prefix can be any arbitrary length from `/0` to `/32`:
- Prefix length $P$ indicates that the first $P$ bits are the Network ID, and the remaining $32 - P$ bits are the Host ID.
- Example: `10.20.30.0/26`:
  - Prefix: 26 bits
  - Subnet Mask: 26 ones followed by 6 zeros = `255.255.255.192`
  - Host bits: $32 - 26 = 6$ bits
  - Total addresses: $2^6 = 64$
  - Usable host addresses: $64 - 2 = 62$

---

## 3. Special IPv4 Address Ranges

| Range | CIDR | Purpose / RFC |
| :--- | :--- | :--- |
| `10.0.0.0` - `10.255.255.255` | `10.0.0.0/8` | **RFC 1918 Private**: Enterprise clouds, large internal LANs |
| `172.16.0.0` - `172.31.255.255` | `172.16.0.0/12` | **RFC 1918 Private**: Docker containers, medium VPCs |
| `192.168.0.0` - `192.168.255.255` | `192.168.0.0/16` | **RFC 1918 Private**: Home routers, small subnets |
| `127.0.0.0` - `127.255.255.255` | `127.0.0.0/8` | **Loopback**: Localhost inter-process communication |
| `169.254.0.0` - `169.254.255.255` | `169.254.0.0/16` | **RFC 3927 APIPA**: Link-local fallback when DHCP fails; AWS IMDS (`169.254.169.254`) |
| `0.0.0.0/0` | `0.0.0.0/0` | **Default Route**: "All networks / Internet" |
| `255.255.255.255/32` | `255.255.255.255/32` | **Limited Broadcast**: Broadcast to local physical segment only |

---

## 4. IPv6: The 128-Bit Successor (RFC 8200)

IPv6 expands the address space from $2^{32} \approx 4.3 \times 10^9$ addresses to:
$$2^{128} \approx 3.4 \times 10^{38} \text{ addresses}$$
(Enough to allocate trillions of IP addresses to every square millimeter of the Earth's surface).

### Representation Rules:
- 8 groups of 4 hexadecimal digits separated by colons: `2001:0db8:85a3:0000:0000:8a2e:0370:7334`.
- **Leading Zeros**: May be omitted: `2001:db8:85a3:0:0:8a2e:370:7334`.
- **Zero Compression (`::`)**: A single contiguous sequence of all-zero groups may be replaced by `::` (can only be used once per address):
  `2001:db8:85a3::8a2e:370:7334`
  Loopback: `::1` (equivalent to `127.0.0.1`).
  Unspecified: `::` (equivalent to `0.0.0.0`).
- Standard end-user subnet allocation: `/64` (providing $2^{64} \approx 18.4 \text{ quintillion}$ addresses per local network).
