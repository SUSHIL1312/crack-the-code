# File Systems & I/O: Top FAANG Interview Questions

### Q1: What happens under the hood when a process calls `write()`? When is the data actually written to disk?
**Answer**:
1. **User to Kernel Transition**: The process invokes `write(fd, buf, count)`. A CPU mode switch transitions execution from Ring 3 to Ring 0.
2. **Virtual File System (VFS)**: The kernel validates the file descriptor `fd` in the process's file descriptor table, retrieves the underlying `struct file`, checks permissions, and calls the file-system-specific write method (`inode->i_fop->write_iter`).
3. **Page Cache Buffer Copy**: For standard I/O (buffered I/O), the kernel allocates or locates physical 4 KB pages in the Page Cache corresponding to the file offsets being written. The data is copied via `memcpy` from user-space memory (`buf`) into kernel page cache memory.
4. **Dirty Page Marking**: The affected page cache pages are marked with the `PG_dirty` flag.
5. **Return**: The `write()` system call returns immediately with the number of bytes copied into RAM—**no disk I/O has occurred yet**.
6. **Flushing to Disk**:
   - Asynchronously, background kernel threads (`kworker` / `flusher` threads in Linux) periodically scan dirty pages (governed by `/proc/sys/vm/dirty_background_ratio` and `/proc/sys/vm/dirty_expire_centisecs`) and flush them to disk via block layer request queues.
   - Synchronously, the application can force immediate flushing using `fsync(fd)` (flushes data and metadata) or `fdatasync(fd)` (flushes data blocks and only metadata necessary to read the file, skipping access time).
   - Alternatively, opening the file with `O_SYNC` or `O_DSYNC` causes every `write()` to block until the disk controller acknowledges durability.

---

### Q2: What is the difference between a Hard Link and a Soft (Symbolic) Link?
**Answer**:
| Feature | Hard Link | Symbolic Link (Soft Link) |
| :--- | :--- | :--- |
| **Inode** | Shares the **exact same inode** as original file. | Has its **own distinct inode** containing path text. |
| **Filesystem Boundaries** | Cannot cross different filesystems/partitions (inode IDs are only unique per filesystem). | Can link across filesystems, network mounts, and non-existent targets. |
| **Directories** | Cannot hard link directories (prevents circular loops in directory tree). | Can freely link to directories. |
| **Target Deletion** | Target file data persists as long as link count > 0 (`st_nlink > 0`). | Becomes a dangling/broken link if target is deleted. |
| **Storage Overhead** | Only adds an entry `(name, inode_num)` in directory data block; 0 extra inodes. | Allocates a new inode and disk block (or stores path in inode fast symlink). |

---

### Q3: How do databases avoid double-buffering when performing file I/O?
**Answer**:
- **The Problem**: Standard OS file I/O copies data from disk to kernel **Page Cache**, and then from Page Cache to user-space application memory (like MySQL InnoDB Buffer Pool or RocksDB block cache). This results in double-buffering: the same 16 KB database page resides in both OS RAM and database RAM, wasting 50% of available memory and causing redundant CPU copies.
- **The Solution**: Direct I/O (`O_DIRECT` flag in `open()`).
  - When opening a file with `O_DIRECT`, the kernel bypasses the Page Cache completely.
  - Data transfers happen directly between user-space memory buffers and the storage controller using DMA (Direct Memory Access).
  - Requirements for `O_DIRECT`: User buffers, file offsets, and transfer lengths must be aligned to physical block sizes (typically 512 bytes or 4096 bytes).
  - High-performance storage engines (RocksDB, Cassandra, ScyllaDB, PostgreSQL, InnoDB) use Direct I/O or asynchronous direct I/O (`io_uring` / `libaio`) to manage their own caching policies (e.g., custom LRU/CLOCK, dirty page flushing) without OS interference.

---

### Q4: Why can `rm -f /var/log/app.log` fail to reclaim disk space?
**Answer**:
- In Unix/Linux, deleting a file with `rm` calls `unlink()`.
- `unlink()` removes the directory entry pointing to the inode and decrements the inode's link count (`st_nlink`).
- The kernel will **only** free the inode and its allocated disk blocks if:
  1. The link count reaches 0 (`st_nlink == 0`), **AND**
  2. The reference count of active open file descriptors across all running processes reaches 0 (`i_count == 0`).
- If an application (e.g. a running Java or C++ logging daemon) still has the file descriptor open, the file data remains on disk and space is **not** freed, even though the filename is invisible in the directory.
- **Resolution**:
  - Identify open deleted files using `lsof | grep deleted` or `fuser`.
  - Restart or signal the holding process, or truncate the file directly via `/proc/<pid>/fd/<fd_num>` using `truncate -s 0 /proc/<pid>/fd/<N>`.
