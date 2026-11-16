# Internal Working: Socket Hashing and RFC 1071 Checksum Algorithm

---

## 1. How the Linux Kernel Demultiplexes TCP Sockets (`inet_hashinfo`)

In high-concurrency systems (like an Nginx or Envoy proxy handling 100,000 concurrent TCP connections), the kernel cannot iterate through an array of 100,000 sockets on every incoming packet.

Inside the Linux kernel networking subsystem (`net/ipv4/inet_hashtables.c`), TCP sockets are maintained in three global hash tables:
1. **Listening Hash Table (`listening_hash`)**: Indexed by `(Destination Port)`. Used only for incoming `SYN` packets initiating new connections.
2. **Established Hash Table (`ehash`)**: Scalable hash table indexed by the full 4-tuple:
   $$\text{hash} = \text{jhash\_3words}(\text{src\_ip}, \text{dst\_ip}, (\text{src\_port} \ll 16) \mid \text{dst\_port}, \text{salt})$$
   - When a packet arrives, the NIC driver or kernel computes this hash in $\approx 10 \text{ nanoseconds}$, indexes directly into an ehash bucket, and obtains the `struct sock*`.
3. **Bind Hash Table (`bhash`)**: Tracks local port allocations for port conflict detection during `bind()`.

---

## 2. The RFC 1071 Internet Checksum Algorithm

Both TCP and UDP use the identical mathematical algorithm: **16-bit Ones' Complement Sum**.

### Why Ones' Complement?
- **End-Around Carry**: When adding two 16-bit integers causes an overflow bit beyond bit 15, the carry bit is added back into the least significant bit (bit 0).
- **Endianness Agnostic**: Whether the calculation is performed on Big-Endian or Little-Endian CPU hardware, the mathematical result is identical! A 16-bit sum of bytes in network order yields the exact same bits when swapped.

### The IPv4 Pseudo-Header:
Crucially, the checksum does **not** only cover the TCP or UDP packet. It includes a **12-byte IPv4 Pseudo-Header** prepended in memory during checksum calculation:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Source IP Address                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Destination IP Address                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Zero      |    Protocol   |        TCP/UDP Length         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
- **Why include the IP Pseudo-Header?**
  - To verify that the packet was not misdelivered to the wrong destination IP address, and that the IP layer protocol field wasn't corrupted in transit.
  - If a router inadvertently corrupts the Destination IP address in the IP header, the transport checksum will fail at the receiver, and the receiver will discard the corrupt packet!

### Verification Property:
When computing the checksum at the sender, the checksum field itself is set to `0x0000`. The inverted sum is placed in the checksum field.
Upon arrival at the receiver, the receiver sums **all words including the received checksum**.
If no bit errors occurred, the sum is **`0xFFFF` (or `~0xFFFF = 0x0000`)**!
