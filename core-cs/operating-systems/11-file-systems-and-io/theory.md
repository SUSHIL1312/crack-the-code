# Theory: Filesystem Architecture, Inodes, and Persistence

## 1. The Three Building Blocks: Inodes, Dentries, and Data Blocks

A modern filesystem (such as Ext4 or APFS) separates file metadata from file naming and file data:

1. **Inode (Index Node)**:
   - The unique integer identity of a file (`st_ino`).
   - Contains all metadata: file size, ownership (UID/GID), permission bits (`rwxr-xr-x`), timestamps (`atime`, `mtime`, `ctime`), and pointers to the physical disk blocks (extents).
   - **Crucial Fact: An Inode does NOT contain the file's name!**
2. **Directory Entry (Dentry)**:
   - A directory is simply a special file that maps human-readable names (`"document.pdf"`) to Inode numbers (`148293`).
3. **Data Blocks**:
   - The actual physical sectors on SSD/HDD holding file bytes.

---

## 2. Hard Links vs Symbolic (Soft) Links

```
A. HARD LINK:
Directory Entry "file.txt" (Inode 501) ──┐
                                         ├──► Inode 501 (Ref Count = 2) ──► Data Blocks
Directory Entry "link.txt" (Inode 501) ──┘

B. SYMBOLIC (SOFT) LINK:
Directory Entry "softlink.txt" (Inode 802) ──► Inode 802 ──► Data: "file.txt" (Path string)
                                                                    │
                                                                    ▼
                                                            Resolves to Inode 501
```

| Dimension | Hard Link | Symbolic (Soft) Link |
| :--- | :--- | :--- |
| **Inode Number** | Shares the **exact same Inode number** as target. | Has its **own unique Inode number**. |
| **Reference Count** | Increments the Inode's `i_nlink` counter. | Does NOT change the target's reference count. |
| **Target Deletion** | Target data is preserved! Data blocks are freed only when link count reaches 0. | Becomes a **Dangling / Broken Link**. |
| **Cross-Filesystem?**| ❌ No (Inode numbers are only unique within a partition). | ✅ Yes (Stores target path as a string). |
| **Links to Directories?**| ❌ Forbidden in most OSes (prevents infinite cycles). | ✅ Yes. |

---

## 3. File Timestamps: `atime`, `mtime`, and `ctime`

- **`mtime` (Modification Time)**: Updated when the **content** of the file is written to or truncated.
- **`atime` (Access Time)**: Updated when the file is read (often disabled via `noatime` in production mounts to avoid disk write overhead on reads).
- **`ctime` (Change Time)**: Updated when the **Inode metadata** changes (e.g. `chmod`, `chown`, moving the file, hard linking).
  - *Classic Interview Trap*: `ctime` is **Change Time**, NOT Creation Time!

---

## 4. The Kernel Page Cache and `fsync()`

When an application calls `write(fd, data, len)`:
1. The kernel copies the bytes from user space into physical DRAM known as the **Page Cache**.
2. The modified pages are marked **Dirty**.
3. The `write()` system call returns **immediately in ~1 microsecond**!
4. **Data is NOT yet on disk!** If the power cord is pulled at this instant, data in the page cache is permanently lost.
5. Background kernel flusher threads (`pdflush`/`flush`) write dirty pages to SSD every few seconds (governed by `dirty_expire_centisecs`).

### How to Guarantee Durability:
- **`fsync(fd)`**: Blocks until all dirty data blocks **and** file metadata (file size, mtime) are physically flushed to non-volatile storage.
- **`fdatasync(fd)`**: Flushes data blocks, but skips metadata flushes unless the file size changed (avoids an extra disk head seek).
- **Direct I/O (`O_DIRECT`)**: Bypasses the OS page cache entirely; user buffer transfers directly to the storage controller via DMA (used by databases like Oracle, MySQL InnoDB, and PostgreSQL to manage their own buffer pools).
