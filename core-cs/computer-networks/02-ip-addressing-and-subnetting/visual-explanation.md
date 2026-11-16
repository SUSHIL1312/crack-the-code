# Visual Explanation: Subnetting & CIDR Slicing

---

## 1. Bitwise Anatomy of a `/26` Subnet

Consider IP: `192.168.1.135/26`

```
Byte 1: 192       Byte 2: 168       Byte 3: 1         Byte 4: 135
1 1 0 0 0 0 0 0   1 0 1 0 1 0 0 0   0 0 0 0 0 0 0 1   1 0 0 0 0 1 1 1

Subnet Mask (/26 = 26 ones, 6 zeros):
1 1 1 1 1 1 1 1   1 1 1 1 1 1 1 1   1 1 1 1 1 1 1 1   1 1 0 0 0 0 0 0
[---------------- Network ID: 26 bits ----------------] [ Host ID: 6 bits ]

Bitwise AND (Network Address):
1 1 0 0 0 0 0 0   1 0 1 0 1 0 0 0   0 0 0 0 0 0 0 1   1 0 0 0 0 0 0 0
192       .       168       .       1         .       128  -> Network: 192.168.1.128

Set Host Bits to 1 (Broadcast Address):
1 1 0 0 0 0 0 0   1 0 1 0 1 0 0 0   0 0 0 0 0 0 0 1   1 0 1 1 1 1 1 1
192       .       168       .       1         .       191  -> Broadcast: 192.168.1.191

First Usable Host: 192.168.1.129
Last Usable Host:  192.168.1.190
Total Hosts: 64 | Usable Hosts: 62
```

---

## 2. Hierarchical Subnet Slicing (VLSM Tree)

Splitting a `/24` network (`10.0.0.0/24`, 256 addresses) into varying sized subnets:

```
                            10.0.0.0/24 (256 addresses)
                                     /             \
                                    /               \
                          10.0.0.0/25               10.0.0.128/25
                        (128 addresses)            (128 addresses)
                        [Engineering VPC]                 /        \
                                                         /          \
                                               10.0.0.128/26      10.0.0.192/26
                                               (64 addresses)     (64 addresses)
                                               [Sales Subnet]            /     \
                                                                        /       \
                                                              10.0.0.192/27   10.0.0.224/27
                                                              (32 addresses)  (32 addresses)
                                                              [Dev Ops]       [DMZ Gateway]
```
Variable Length Subnet Masking (VLSM) avoids wasted space by tailoring the prefix length to each team's exact capacity requirements.
