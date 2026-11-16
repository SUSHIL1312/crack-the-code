# Visual Explanation: TCP State Transitions and Congestion Control Windows

---

## 1. TCP Finite State Machine (FSM)

```
                              +---------+
                              | CLOSED  |
                              +---------+
                               /       \
                        Active  /         \ Passive Open
                         Open  /           \ (listen())
                              v             v
                         +----------+  +--------+
                         | SYN_SENT |  | LISTEN |
                         +----------+  +--------+
                              |             |
                     Receive  |     Receive |
                     SYN+ACK  |     SYN     | Send SYN+ACK
                     Send ACK v             v
                         +---------------+
                         |  SYN_RECEIVED |
                         +---------------+
                                 |
                                 | Receive ACK
                                 v
                       +-------------------+
                       |    ESTABLISHED    | <== Normal Data Transfer
                       +-------------------+
                         /               \
            Active Close/                 \ Passive Close
            (Send FIN) /                   \ (Receive FIN, Send ACK)
                      v                     v
              +--------------+      +--------------+
              |  FIN_WAIT_1  |      |  CLOSE_WAIT  |
              +--------------+      +--------------+
                      |                     |
             Receive  |             Close() |
             ACK      |             Send FIN|
                      v                     v
              +--------------+      +--------------+
              |  FIN_WAIT_2  |      |   LAST_ACK   |
              +--------------+      +--------------+
                      |                     |
             Receive  |             Receive |
             FIN      |             ACK     |
             Send ACK v                     v
              +--------------+      +--------------+
              |  TIME_WAIT   |      |    CLOSED    |
              | (Wait 2*MSL) |      +--------------+
              +--------------+
                      |
              Timer   |
              Expires v
              +--------------+
              |    CLOSED    |
              +--------------+
```

---

## 2. Congestion Window (cwnd) Evolution: Reno vs Cubic vs BBR

```
Congestion Window (cwnd in MSS)
  ^
  |                     / \                      / \           (TCP Reno: Sawtooth AIMD)
  |       Slow        /     \    Congestion    /     \
  |       Start     /         \  Avoidance   /         \
  |        /\     /             \  (AIMD)   /             \
  |       /  \  /                 \       /                 \
  |      /    \/                    \   /                     \
  |     /                            \ /                        \
  +----+-------------------------------------------------------------> Time
        ^      ^                      ^
        |      |                      |
     IW=10   ssthresh              Packet Drop (Fast Retransmit)
             reached               ssthresh = cwnd/2, cwnd = ssthresh


  ^
  |                   .---.                    .---.          (TCP Cubic: Cubic Growth Curve)
  |                  /     \                  /     \
  |       Slow      /       \                /       \
  |       Start    /  Rapid  \              /  Rapid  \
  |        /\     /   scale   \            /   scale   \
  |       /  \  /     up       \          /     up      \
  |      /    \/                \        /               \
  +----+-------------------------------------------------------------> Time


  ^
  |    ========================================================  (Google BBR: Paced Transmission)
  |    (Maintains cwnd = BDP without forming bufferbloat queue!)
  +------------------------------------------------------------------> Time
```
Notice how TCP Reno produces a jagged, inefficient sawtooth pattern, TCP Cubic rapidly returns to peak bandwidth using a cubic plateau, and Google BBR eliminates the sawtooth entirely by pacing packets to match the physical link rate!
