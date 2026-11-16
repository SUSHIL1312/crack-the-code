# Visual Explanation: Inodes, Links & Persistence Guarantees

## 1. Hard Links vs Soft Links Directory Architecture

```
Directory "my_folder/"
┌───────────────────────────────┬───────────────────────────────┐
│ Entry Name                    │ Target Inode                  │
├───────────────────────────────┼───────────────────────────────┤
│ "original.txt"                │ Inode 1042                    │
│ "hard_link.txt"               │ Inode 1042 (Exact same inode!)│
│ "symlink.txt"                 │ Inode 9999 (New inode)        │
└───────────────────────────────┴───────────────────────────────┘

Inode 1042 Metadata:
┌───────────────────────────────────────────────────────────────┐
│ Inode Number: 1042                                            │
│ Link Count (i_nlink): 2                                       │
│ Size: 4096 Bytes                                              │
│ Data Extents: Disk Block 88510 ──► [ "Hello World Data" ]     │
└───────────────────────────────────────────────────────────────┘

Inode 9999 (Symlink Metadata):
┌───────────────────────────────────────────────────────────────┐
│ Inode Number: 9999                                            │
│ Link Count: 1                                                 │
│ Type: S_IFLNK (Symlink)                                       │
│ Data: "original.txt" (String path reference)                  │
└───────────────────────────────────────────────────────────────┘
```

---

## 2. Durability Timeline: `write()` vs `fsync()`

```
Time   Application                 Kernel Page Cache (DRAM)         NVMe SSD Storage
 │
 ▼     write(fd, "Data", 4)
       (Fast! ~1 microsecond) ───► [ Stored in Page Cache ]
                                   [ Marked as DIRTY ]
                                   (Returns success to app!)
                                                                    (Flash memory is UNTOUCHED!)
       ────────────────────── IF POWER DIES HERE: DATA LOST! ───────────────────────
       
       fsync(fd)
       (Blocks application...)
                                   Kernel flushes dirty blocks ───► Physical Flash Write
                                                                    Drive controller caches flushed
       fsync() returns! ◄────────────────────────────────────────── Persistent on Non-Volatile Silicon!
```
