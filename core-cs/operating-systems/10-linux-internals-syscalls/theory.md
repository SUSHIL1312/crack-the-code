# Theory: Linux Internals, File Descriptors, and System Calls

## 1. The Virtual Filesystem (VFS)

In UNIX and Linux, the foundational design philosophy is: **"Everything is a File."**
Whether interacting with a regular file on an ext4 SSD, a TCP network socket, a terminal tty, a named pipe, or a kernel metric in `/proc`, the application issues the exact same system calls: `read()`, `write()`, `close()`.

The **Virtual Filesystem (VFS)** is the kernel abstraction layer that makes this possible:

```
User Applications (read / write / close)
                      │
                      ▼
         Virtual Filesystem Switch (VFS)
                      │
     ┌────────────────┼────────────────┬────────────────┐
     ▼                ▼                ▼                ▼
Ext4 / XFS Filesystem Socket Layer   Pipes / FIFOs     /proc Pseudo-FS
(Physical Disk)       (NIC / Network)  (Inter-Process)   (Kernel Memory)
```

---

## 2. File Descriptors & The Three-Tier Architecture

When an application calls `int fd = open("/path/file.txt", O_RDWR);`, three separate data structures in kernel memory coordinate the file access:

```
Process A (task_struct)
┌──────────────────────┐
│ File Descriptor Table│
│ fd 0: stdin          │
│ fd 1: stdout         │
│ fd 3: ───────────────┼──────────┐
└──────────────────────┘          │
                                  ▼
Process B (task_struct)   ┌───────────────────────────────┐
┌──────────────────────┐  │ System-Wide Open File Table   │
│ File Descriptor Table│  │ (struct file)                 │
│ fd 4: ───────────────┼─►│ - File Offset (e.g. 1024)     │
└──────────────────────┘  │ - Access Mode (O_RDWR)        │
                          │ - Reference Count = 2         │
                          │ - Pointer to Inode ───────────┼──────────┐
                          └───────────────────────────────┘          │
                                                                     ▼
                                                          ┌──────────────────────┐
                                                          │ Inode Table (VFS)    │
                                                          │ (struct inode)       │
                                                          │ - File Size          │
                                                          │ - Permissions & Owner│
                                                          │ - Disk Block Pointers│
                                                          └──────────────────────┘
```

1. **Per-Process File Descriptor Table** (`files_struct`): An array of pointers indexed by the integer `fd`.
2. **System-Wide Open File Description** (`struct file`): Represents an active open file session. Stores the current **file offset** (where the next `read` or `write` will occur) and open status flags.
   - *Key Insight*: When `fork()` is called, the child inherits duplicate file descriptors that point to the **same open file description**, meaning parent and child share the same file offset!
3. **VFS Inode** (`struct inode`): Represents the concrete file on disk. Stores file metadata (size, owner, permissions, creation timestamp) and pointers to storage blocks.

---

## 3. I/O Multiplexing Evolution: `select` vs `poll` vs `epoll`

To handle 100,000 concurrent network connections without creating 100,000 threads, modern servers use **I/O Multiplexing**:

| Metric | `select()` (1983) | `poll()` (1997) | `epoll()` (Linux 2.5.44+, 2002) |
| :--- | :--- | :--- | :--- |
| **Data Structure** | Fixed-size bitmask (`fd_set`, max 1024 FDs). | Dynamically sized array of `struct pollfd`. | In-kernel **Red-Black Tree** (interest list) and **Ready List** (doubly linked list). |
| **Complexity** | $O(N)$ linear scan of all 1024 bits on every call. | $O(N)$ linear scan of all passed FDs. | **$O(1)$ Event-Driven**. Only returns file descriptors that are actually ready! |
| **System Call Cost** | User must copy entire bitmask to kernel and back on *every single invocation*. | User must copy entire array to kernel on every invocation. | FDs are registered once via `epoll_ctl()`. `epoll_wait()` only returns active events. |
| **Scalability** | Collapses under > 1,000 connections. | Slow under > 10,000 connections. | **Scales easily to 1,000,000+ connections** (The C10K / C1000K solution). |
