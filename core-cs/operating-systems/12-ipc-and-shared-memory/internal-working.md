# Internal Working: Inter-Process Communication & Kernel Subsystems

Understanding the exact kernel data structures and physical memory mappings behind IPC.

---

## 1. Page Table Mapping in Shared Memory (`MAP_SHARED`)

When two processes call `mmap()` with `MAP_SHARED` on the same file descriptor (e.g. from `shm_open`):

```
Process A Virtual Address Space           Process B Virtual Address Space
   [ 0x7fff'1000'0000 ]                      [ 0x7fff'5000'0000 ]
           |                                         |
           v                                         v
   Process A Page Table                      Process B Page Table
  [ VPN_A -> PPN 0x4A10 ]                   [ VPN_B -> PPN 0x4A10 ]
           \                                         /
            \                                       /
             +-------------> Physical RAM <--------+
                            [ PPN 0x4A10 ]
                            (Single Copy)
```

1. **Different Virtual, Identical Physical**: Virtual addresses in Process A and Process B can be completely different (and almost always are due to ASLR). But their page table entries (PTEs) both resolve to the identical physical page frame (`PPN 0x4A10`).
2. **CPU Cache Coherence**: Because both CPUs access the same physical cache lines, hardware MESI/MOESI cache coherence protocols keep CPU L1/L2 caches synchronized across multi-core sockets automatically.
3. **Dirty Bits and tmpfs**: In POSIX shared memory, pages reside in physical RAM backed by a RAM disk (`tmpfs`). No swap or block device flushing occurs unless memory pressure forces paging.

---

## 2. Kernel Pipe Implementation (`struct pipe_inode_info`)

Inside the Linux kernel, a pipe is not an on-disk file; it is an in-memory circular buffer managed by `struct pipe_inode_info`:

```
+-------------------------------------------------------------------------+
|                        struct pipe_inode_info                           |
|  - head: int (write index)                                              |
|  - tail: int (read index)                                               |
|  - ring_size: unsigned int (default 16 pages = 64 KB)                   |
|  - bufs: struct pipe_buffer[16]                                         |
|  - r_wait: wait_queue_head_t (processes waiting to read)                |
|  - w_wait: wait_queue_head_t (processes waiting to write)               |
+-------------------------------------------------------------------------+
```

- When Process A writes to `pipefd[1]`:
  1. The kernel acquires the pipe mutex.
  2. Copies data from Process A's user space buffer into the pipe's internal page array (`pipe_buffer`).
  3. Increments `head`.
  4. Wakes up any processes sleeping on `r_wait` using `wake_up_interruptible()`.
  5. If the pipe becomes full, Process A is placed onto `w_wait` and context-switched out (`schedule()`).

---

## 3. Passing File Descriptors across Processes (`SCM_RIGHTS`)

A file descriptor is simply an integer index into a process's private `struct files_struct` array (`fd_array`). Passing the integer `4` via a standard pipe is useless because file descriptor `4` in Process B points to whatever Process B happens to have opened.

To truly share or transfer an open file descriptor:
1. Sender uses `sendmsg()` over a Unix Domain Socket with an `msghdr` containing control message `cmsg_type = SCM_RIGHTS` holding `int fd`.
2. The kernel intercepts the control message:
   - Resolves sender's `fd` to the underlying kernel `struct file*`.
   - Increments the reference count of the `struct file` (`file->f_count++`).
   - Packages the `struct file*` pointer in the socket buffer `sk_buff`.
3. When Receiver calls `recvmsg()`:
   - Kernel allocates the lowest unused file descriptor integer in **Receiver's** `fd_array`.
   - Points this new integer to the **same** underlying `struct file*`.
   - Passes the new integer to Receiver in the ancillary buffer.

---

## 4. Robust Process-Shared Mutexes (`PTHREAD_MUTEX_ROBUST`)

What happens when Process A acquires a process-shared mutex located in shared memory and is terminated with `kill -9` (`SIGKILL`)?
- Without robustness: The mutex remains locked forever. Any other process attempting to acquire it blocks indefinitely (deadlock).
- With `PTHREAD_MUTEX_ROBUST`:
  1. The kernel maintains a robust list pointer (`set_robust_list`) for every thread.
  2. When the kernel terminates the thread, it traverses the robust list.
  3. For every mutex held, the kernel sets the `FUTEX_OWNER_DIED` bit in the mutex's futex word and wakes up waiting threads.
  4. The next process calling `pthread_mutex_lock()` returns `EOWNERDEAD`.
  5. The application can inspect/repair state consistency and call `pthread_mutex_consistent()`, making the lock normal again.
