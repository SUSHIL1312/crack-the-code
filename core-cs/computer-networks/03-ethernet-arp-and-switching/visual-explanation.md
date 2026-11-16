# Visual Explanation: ARP Resolution and Switch MAC Learning

---

## 1. Visual Walkthrough: How Host A Resolves Host B via ARP

```
Host A (192.168.1.10)                                                   Host B (192.168.1.20)
MAC: AA:AA:AA:AA:AA:AA                                                  MAC: BB:BB:BB:BB:BB:BB
        |                                                                       |
        | 1. ARP Request:                                                       |
        |    "Who has 192.168.1.20? Tell 192.168.1.10"                         |
        |    Dst MAC: FF:FF:FF:FF:FF:FF (Broadcast)                             |
        +======================> [ L2 Switch ] =================================>
                                       |
                                       | Floods broadcast to all ports!
                                       v
                             [ Host C (192.168.1.30) ]
                             (Not my IP -> Drops frame)
                                                                                |
                                                                                | 2. Host B learns:
                                                                                |    192.168.1.10 is at
                                                                                |    AA:AA:AA:AA:AA:AA
                                                                                |
                                                                                | 3. ARP Reply:
                                                                                |    "192.168.1.20 is at
                                                                                |     BB:BB:BB:BB:BB:BB"
                                                                                |    Dst MAC: AA:AA:AA:AA:AA:AA
        <----------------------- [ L2 Switch ] <--------------------------------+
        |                        (Unicast directly to Port 1!)
        v
   Host A learns:
   192.168.1.20 is at
   BB:BB:BB:BB:BB:BB
        |
        | 4. IP Packet Transmission:
        |    Dst MAC: BB:BB:BB:BB:BB:BB | Dst IP: 192.168.1.20
        +-----------------------------------------------------------------------> (Data Arrives!)
```

---

## 2. Switch CAM Table Transition Lifecycle

```
Scenario: Fresh Switch Boot (Empty CAM Table)
=============================================================================
Step 1: Host A (MAC: AA) sends frame to Host B (MAC: BB)
  - Switch learns: AA is on Port 1
  - Destination BB is UNKNOWN -> Floods out Ports 2, 3, 4 (Unknown Unicast Flood)
  CAM Table:
    [ MAC: AA -> Port 1 ]

Step 2: Host B (MAC: BB) replies to Host A (MAC: AA)
  - Switch learns: BB is on Port 2
  - Destination AA is KNOWN (Port 1) -> Forwards ONLY to Port 1! (Zero flooding)
  CAM Table:
    [ MAC: AA -> Port 1 ]
    [ MAC: BB -> Port 2 ]

Step 3: Host A sends next frame to Host B
  - Destination BB is KNOWN (Port 2) -> Forwards ONLY to Port 2. (Clean switching)
=============================================================================
```
