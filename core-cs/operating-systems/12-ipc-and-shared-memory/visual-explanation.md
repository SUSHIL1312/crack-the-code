# Visual Explanation: Inter-Process Communication

Visualizing the data paths, latency differences, and buffer mechanisms across IPC mechanisms.

---

## 1. Data Path Comparison: Kernel Copy vs Zero-Copy

### Pipe / Unix Domain Socket (Two Kernel Memory Copies)
```
[ Process A (User Space) ]
       |  write(fd, user_buf, N)
       v
+------------------------------------+
|  Copy #1: user_buf -> sk_buff      |   <-- Ring 0 Kernel Space
|  Kernel Socket / Pipe Buffer (RAM) |
+------------------------------------+
       |  read(fd, recv_buf, N)
       v
[ Process B (User Space) ]  <-- Copy #2: sk_buff -> recv_buf
```

### POSIX Shared Memory (Zero Copies)
```
[ Process A (User Space) ]           [ Process B (User Space) ]
  ptrA[0] = 42;                        x = ptrB[0];
       \                                   /
        \                                 /
         v                               v
    +-----------------------------------------+
    |           Physical RAM Page             |  <-- Single Shared Frame
    |           [ PPN 0x7E30 ]                |
    +-----------------------------------------+
```

---

## 2. Lock-Free SPSC Shared Memory Ring Buffer Architecture

```
Shared Memory Region (mmap)
+---------------------------------------------------------------------------------+
|  head (atomic<size_t>)  |  tail (atomic<size_t>)  |  buffer[CAPACITY] (slots)   |
|  (only written by       |  (only written by       |                             |
|   Producer)             |   Consumer)             |                             |
+---------------------------------------------------------------------------------+

                      Write Head -> [ Slot 3 ]
                           |
                           v
    [ Slot 0 ] -> [ Slot 1 ] -> [ Slot 2 ] -> [ Slot 3 ] -> [ Slot 4 ] ...
                     ^
                     |
               Read Tail -> [ Slot 1 ]

- Producer calculates: (head + 1) % CAPACITY != tail (check for buffer full)
- Writes data to buffer[head]
- Updates head with memory_order_release
- Consumer reads head with memory_order_acquire
- Reads data from buffer[tail]
- Updates tail with memory_order_release
```

---

## 3. Passing File Descriptors across Processes via `sendmsg`

```
Process A (PID 1001)                               Process B (PID 1002)
fd_table:                                          fd_table:
  [0] -> stdin                                       [0] -> stdin
  [1] -> stdout                                      [1] -> stdout
  [2] -> stderr                                      [2] -> stderr
  [3] -> socket_A  ===============================>  [3] -> socket_B
  [4] -> file_X --------------------+                [4] -> file_X  (Assigned by kernel!)
                                    |                      |
                                    v                      |
                            +--------------------+         |
                            |  struct file (X)   |<--------+
                            |  f_count: 2        |
                            |  f_pos: 1024       |
                            |  f_inode: 0x9812   |
                            +--------------------+
```
Both processes now share the exact same open file description, meaning file offsets and lock states are synchronized in real time!
