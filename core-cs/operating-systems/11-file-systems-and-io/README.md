# Module 11: File Systems & Storage I/O

<!-- Chapter Navigation Header -->
| ⬅️ [Module 10: Linux Syscalls](../10-linux-internals-syscalls/README.md) | 🏠 [Operating Systems Hub](../README.md) | [Module 12: IPC & Shared Memory](../12-ipc-and-shared-memory/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Understanding how storage drives, filesystems, and the operating system page cache interact is critical for building reliable databases, caching tiers, and distributed storage systems.

---

## 📑 Module Contents

1. [Theory & Filesystem Architecture](theory.md)
   * The Anatomy of a File: Inodes, Directory Entries (dentries), and Data Blocks.
   * Hard Links vs Symbolic (Soft) Links.
   * File Metadata: `atime`, `mtime`, and `ctime` (Why `ctime` is NOT creation time!).
   * The Kernel Page Cache, Dirty Pages, and Flusher Daemons.
   * Persistence Guarantees: `write()` vs `fsync()` vs `fdatasync()`.
   * Direct I/O (`O_DIRECT`) for database storage engines.
2. [Internal Working](internal-working.md)
   * Linux Ext4 Inode structure and Extents tree.
   * The `fsync()` write pipeline: From user buffer to DRAM page cache to drive non-volatile flash.
   * Modern Linux Asynchronous I/O: `io_uring` ring-buffer queues.
3. [Visual Explanations](visual-explanation.md)
   * Hard links pointing to the same Inode vs Soft links containing path strings.
   * The `write()` vs `fsync()` durability timeline.
4. [Examples & Code](examples/)
   * `hard_vs_soft_links.cpp`: Programmatically creates hard and soft links, inspecting inode numbers and link counts via `stat()`.
   * `buffered_vs_fsync_bench.cpp`: Measures the dramatic latency gap between buffered page-cache writes (~1 $\mu$s) vs durable `fsync()` disk flushes (~5 ms).
5. [Exercises & Solutions](exercises/exercises.md)
   * Recovering unlinked open files from `/proc/<pid>/fd/`, designing write-ahead logging (WAL).
6. [Interview Questions & Deep Dives](interview-questions.md)
   * What happens when you delete an open file? `fsync` vs `fdatasync`, Hard vs Soft links, why databases use Direct I/O.
7. [Common Mistakes](common-mistakes.md)
   * Assuming `write()` persists data to disk, ignoring `fsync()` error codes.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 10: Linux Syscalls](../10-linux-internals-syscalls/README.md) | [Operating Systems Hub](../README.md) | **[Module 12: IPC & Shared Memory](../12-ipc-and-shared-memory/README.md)** |
<!-- /Chapter Navigation Footer -->

