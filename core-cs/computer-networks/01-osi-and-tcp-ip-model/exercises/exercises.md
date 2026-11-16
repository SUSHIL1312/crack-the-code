# Hands-On Exercises: OSI & TCP/IP Layer Models

---

### Exercise 1: UDP DNS Packet Dissection
**Goal**: Extend the packet dissector to handle UDP packets (Protocol 17) and dissect a raw DNS query packet.

**Requirements**:
1. Identify the 8-byte UDP header (Source Port, Destination Port, Length, Checksum).
2. Recognize Destination Port 53 (DNS).
3. Parse the DNS Header (Transaction ID, Flags, Questions count, Answer RRs count).
4. Extract the queried domain name (decoding length-prefixed DNS labels, e.g. `\x06google\x03com\x00`).

---

### Exercise 2: Maximum Transmission Unit (MTU) & IP Fragmentation
**Goal**: Calculate packet overhead and fragmentation boundaries.

**Scenario**:
An application sends a 4,000-byte UDP payload across an Ethernet network with an MTU of 1,500 bytes.
1. What is the total length of the unfragmented IP packet (IPv4 20-byte header + UDP 8-byte header + 4,000 bytes data)?
2. How many fragments must the IP layer generate?
3. For each fragment, specify:
   - Total Length field in IPv4 header
   - More Fragments (MF) flag (0 or 1)
   - Fragment Offset field value (remember: offset is measured in 8-byte units!)
   - Does fragment #2 contain a UDP header? Explain why or why not.

---

### Exercise 3: Encapsulation Efficiency Calculation
**Goal**: Calculate the payload protocol efficiency for tiny messages.

**Scenario**:
A real-time multiplayer game sends a 4-byte position update `(x, y)`:
- Transport Layer: TCP (20 bytes header, no options)
- Network Layer: IPv4 (20 bytes header)
- Data Link Layer: Ethernet II (14 bytes header + 4 bytes CRC trailer)
1. Calculate total bytes transmitted across the physical wire for one 4-byte update.
2. What is the protocol efficiency percentage ($\frac{\text{Payload Bytes}}{\text{Total Bytes}} \times 100\%$)?
3. If the game switches from TCP to UDP (8 bytes header), how does the efficiency change?
