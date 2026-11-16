# Internal Working: Unlink Mechanics, Ext4 Extents, and `io_uring`

## 1. What Happens Internally When You Delete a File (`unlink`)?

In UNIX, there is no system call named `delete`. The system call is **`unlink(pathname)`**:

```c
int unlink(const char *pathname);
```

### The Exact Kernel Sequence:
1. The kernel looks up the directory entry corresponding to `pathname` and removes the mapping.
2. The kernel decrements the target Inode's link count: `inode->i_nlink--`.
3. **The Two-Pronged Reclamation Check**:
   - The kernel checks: Is `i_nlink == 0` AND are there **zero active open file descriptors** pointing to this inode?
   - **If YES**: The kernel deallocates the physical data blocks, clears the Inode bitmap in the block group descriptor, and returns the storage to the free space pool.
   - **If NO (A process still has the file open)**:
     - The file name disappears from `ls` and the directory.
     - **However, the Inode and physical data blocks remain in RAM and on disk!**
     - The process that holds the open file descriptor can continue to `read()` and `write()` to the invisible file completely unimpeded!
     - Only when the process explicitly calls `close(fd)` or terminates does the kernel finally deallocate the physical disk blocks.

---

## 2. Ext4 Inode & Extents Architecture

Early filesystems used direct, indirect, and triple-indirect block pointers. A large file required thousands of pointers just to record which disk blocks it used.

Ext4 introduced **Extents**:
- An extent maps a continuous range of logical file blocks to contiguous physical disk blocks:
  $$\text{Extent} = (\text{Logical Block Number}, \text{Length in Blocks}, \text{Physical Block Start})$$
- A single extent can represent up to **128 MB of contiguous storage** in just 12 bytes!
- Extents are indexed via a compact B-tree rooted directly inside the Inode structure (`struct ext4_extent_header`).

---

## 3. High-Performance I/O Evolution: `io_uring`

Historically, asynchronous I/O in Linux (`aio_read`/`aio_write`) was flawed: it only worked for unbuffered `O_DIRECT` files and frequently blocked.

In Linux 5.1 (2019), Jens Axboe introduced **`io_uring`**:
- Solves the fundamental bottleneck of high-speed NVMe drives: **System call overhead**.
- Operates via **Two Lock-Free Ring Buffers** mapped in shared memory between user space and the kernel:

```
User Space                                                     Kernel Space
┌──────────────────────────────────────┐                       ┌──────────────────────────────────────┐
│  Submission Queue (SQ Ring)          │                       │  Kernel Worker / Polling Thread      │
│  [ SQE 1 | SQE 2 | SQE 3 ... ]       │───No Syscalls!───────►│  Consumes SQEs, submits to NVMe      │
├──────────────────────────────────────┤   (Shared Memory)     ├──────────────────────────────────────┤
│  Completion Queue (CQ Ring)          │                       │  Direct DMA to User Memory           │
│  [ CQE 1 | CQE 2 ... ]               │◄──Completed Results───│  Pushes CQEs to ring buffer          │
└──────────────────────────────────────┘                       └──────────────────────────────────────┘
```

1. **Submission Queue (SQ)**: The user application writes I/O requests (SQEs) directly to the ring buffer in user space.
2. **Kernel Polling (`IORING_SETUP_SQPOLL`)**: A dedicated kernel thread continuously polls the SQ ring buffer. **Zero system calls are issued!**
3. **Completion Queue (CQ)**: When SSD hardware completes the DMA transfer, the kernel posts a CQE to the ring.
4. **Performance**: Achieves over **3,000,000 IOPS per core**, completely outpacing traditional POSIX I/O.
