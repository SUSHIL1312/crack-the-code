# Common Mistakes in TCP Internals

### 1. Forgetting `SO_REUSEADDR` on Server Sockets
- **Mistake**: Restarting a server daemon immediately after a crash or deploy, and receiving error `bind: Address already in use`.
- **Reality**: The previous instance of the server performed an active close, leaving the listening port or bound 4-tuples in the `TIME_WAIT` state for 60 seconds.
- **Fix**: Set `SO_REUSEADDR` before calling `bind()`:
  ```cpp
  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  ```

---

### 2. Disabling `TIME_WAIT` with Abortive `SO_LINGER = 0`
- **Mistake**: Setting `struct linger l = {1, 0}; setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));` to bypass `TIME_WAIT`.
- **Reality**: Setting linger time to 0 sends an immediate `RST` (connection reset) packet rather than a standard `FIN`. This forcefully aborts the TCP connection, drops unread data in the receiver's queue, and breaks graceful application protocol shutdown.

---

### 3. Assuming Sequence Numbers Count Packets
- **Mistake**: Thinking that if a sender transmits Packet 1 with `Seq = 100`, Packet 2 will have `Seq = 101`.
- **Reality**: TCP is a byte-oriented protocol. Sequence numbers track **byte offsets**. If Packet 1 contains 1,460 bytes of data, Packet 2 has sequence number `Seq = 100 + 1460 = 1560`.

---

### 4. Overlooking Bufferbloat in Cloud Deployments
- **Mistake**: Using legacy loss-based congestion control (TCP Reno / default Cubic) on lossy Wi-Fi or satellite uplinks, experiencing huge 500ms ping spikes.
- **Reality**: Loss-based algorithms keep pumping data into intermediate router buffers until a queue overflows and drops packets.
- **Fix**: Switch the Linux kernel congestion control algorithm to Google BBR:
  ```bash
  sysctl -w net.ipv4.tcp_congestion_control=bbr
  ```
