# Hands-On Exercises: The Transport Layer

---

### Exercise 1: Ephemeral Port Exhaustion Benchmark & Calculation
**Goal**: Analyze client-side socket scalability when calling upstream microservices.

**Scenario**:
A payment gateway service running on Linux issues outbound HTTP/1.1 calls without HTTP Keep-Alive (closing each connection after 1 request) to an upstream credit card processing API at `203.0.113.10:443`.
- Linux default ephemeral port range: `32768` to `60999`.
- TIME_WAIT duration on Linux: 60 seconds (`TCP_TIMEWAIT_LEN`).
1. How many total ephemeral ports are available in this default range?
2. If the service makes 500 requests per second, calculate how many seconds elapse before all ephemeral ports are trapped in the `TIME_WAIT` state.
3. What error will the application receive from `connect()` when ports are exhausted?
4. What three engineering solutions permanently eliminate this bottleneck?

---

### Exercise 2: Manual RFC 1071 Checksum Math
**Goal**: Calculate the 16-bit ones' complement sum by hand to master the bitwise arithmetic.

**Data**:
Given three 16-bit binary words:
- Word A: `0x66B7` (`0110 0110 1011 0111`)
- Word B: `0x4CEF` (`0100 1100 1110 1111`)
- Word C: `0xA123` (`1010 0001 0010 0011`)
1. Show step-by-step addition with end-around carry.
2. Invert all bits to find the final checksum value in hex.
3. Add the checksum back to the original three words and prove that the receiver's sum equals `0xFFFF`.
