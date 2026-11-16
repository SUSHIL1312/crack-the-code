# Visual Explanation: Port Demultiplexing & Ones' Complement Addition

---

## 1. TCP 4-Tuple Demultiplexing Architecture

```
Web Client 1                                                              Web Server (80.80.80.80)
IP: 10.0.0.1                                                              +-------------------------------+
Ephemeral Port: 55000                                                     | Listening Socket: (Port 80)   |
  \                                                                       +-------------------------------+
   \  TCP SYN -> (10.0.0.1:55000 -> 80.80.80.80:80)                                      |
    \                                                                                    v accept()
     +===============================================================> [ Socket 1: (10.0.0.1:55000, 80) ]
                                                                       (Dedicated buffer for Client 1)

Web Client 2
IP: 20.0.0.2
Ephemeral Port: 55000  (Notice: Same client port number!)
  \
   \  TCP SYN -> (20.0.0.2:55000 -> 80.80.80.80:80)
    \                                                                                    v accept()
     +===============================================================> [ Socket 2: (20.0.0.2:55000, 80) ]
                                                                       (Dedicated buffer for Client 2)
```

Because TCP demultiplexes on the complete **4-tuple**, Client 1 and Client 2 can use the identical local port `55000` to connect to port `80` on the server without any ambiguity or packet collision!

---

## 2. Ones' Complement Addition and End-Around Carry

```
Summing 16-bit Words:
       Word 1:  1101 0101  0100 0011  (0xD543)
    +  Word 2:  0111 0010  1011 0110  (0x72B6)
    -------------------------------------------
     Carry = 1  0100 0111  1111 1001  (0x147F9)
                \                     /
                 +---> Add carry back:
                       0100 0111  1111 1001
                    +                     1
                    -----------------------
                       0100 0111  1111 1010  (0x47FA)

    Invert all bits (~sum) = Checksum:
                       1011 1000  0000 0101  (0xB805)
```
When the receiver receives `0xD543`, `0x72B6`, and the checksum `0xB805`, summing all three results in `0xFFFF`. Inverting `0xFFFF` gives `0x0000` (Verification Success!).
