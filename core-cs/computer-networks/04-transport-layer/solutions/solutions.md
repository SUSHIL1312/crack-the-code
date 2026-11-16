# Solutions: The Transport Layer Exercises

---

### Solution 1: Ephemeral Port Exhaustion

1. **Available Ports**:
   $$\text{Ports} = 60,999 - 32,768 + 1 = \mathbf{28,232 \text{ ports}}$$

2. **Time to Exhaustion**:
   - Each closed TCP connection enters the `TIME_WAIT` state for $60 \text{ seconds}$.
   - During those 60 seconds, the 4-tuple `(client_ip, client_port, 203.0.113.10, 443)` cannot be reused.
   - At a rate of 500 requests per second:
     $$\text{Time to exhaust} = \frac{28,232 \text{ ports}}{500 \text{ req/sec}} \approx \mathbf{56.46 \text{ seconds}}$$
   - Because $56.46 < 60 \text{ seconds}$, all 28,232 ports become trapped in `TIME_WAIT` before the first port can expire and be reclaimed!

3. **Application Error**:
   - The OS `connect()` system call returns `-1` with `errno = EADDRNOTAVAIL` (*"Cannot assign requested address"*).

4. **Three Production Solutions**:
   1. **HTTP Keep-Alive / Connection Pooling**: Reuse persistent TCP connections across requests rather than opening and closing a new socket per call (e.g. HTTP client connection pool with 50-100 warm sockets).
   2. **Enable TCP Tw Reuse**: Set sysctl `net.ipv4.tcp_tw_reuse = 1` in Linux. Allows the kernel to safely reuse `TIME_WAIT` sockets for outgoing client connections when RFC 1323 TCP timestamps prove that incoming packets are not old duplicates.
   3. **Expand Port Range & Multi-IP Binding**:
      - Expand local port range: `sysctl -w net.ipv4.ip_local_port_range="1024 65535"` (adds 36,000 extra ports).
      - Bind outbound requests across multiple local virtual IPs. Since the 4-tuple includes the client IP, adding 4 client IPs multiplies ephemeral port capacity by 4x.

---

### Solution 2: Manual RFC 1071 Checksum Calculation

- Word A: `0x66B7`
- Word B: `0x4CEF`
- Word C: `0xA123`

1. **Addition Step 1 (Word A + Word B)**:
   ```
     0x66B7  =  0110 0110 1011 0111
   + 0x4CEF  =  0100 1100 1110 1111
   ---------------------------------
     0xB3A6  =  1011 0011 1010 0110 (No carry out)
   ```

2. **Addition Step 2 (Sum + Word C)**:
   ```
     0xB3A6  =  1011 0011 1010 0110
   + 0xA123  =  1010 0001 0010 0011
   ---------------------------------
    0x154C9  = 1 0101 0100 1100 1001 (Carry bit = 1!)
   ```

3. **End-Around Carry Addition**:
   ```
     0x54C9
   +      1
   --------
     0x54CA  =  0101 0100 1100 1010
   ```

4. **Ones' Complement Inversion (~Sum)**:
   $$\sim \text{0x54CA} = \mathbf{0xAB35} \quad (\texttt{1010 1011 0011 0101})$$
   **The transmitted Checksum is `0xAB35`**.

5. **Receiver Verification**:
   ```
     0x54CA  (Sum of original three words)
   + 0xAB35  (Received Checksum)
   --------
     0xFFFF  (All 16 bits are 1)
   ```
   Taking the bitwise NOT: $\sim \text{0xFFFF} = \mathbf{0x0000}$. Checksum verifies perfectly.
