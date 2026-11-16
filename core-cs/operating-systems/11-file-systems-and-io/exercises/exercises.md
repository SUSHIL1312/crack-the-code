# Hands-On Exercises: File Systems and I/O

Practice these exercises to master file descriptors, inode manipulation, page cache dynamics, and direct I/O.

---

### Exercise 1: Recursive Inode and File Metadata Inspector
**Goal**: Write a tool that traverses a directory hierarchy using POSIX `opendir()`, `readdir()`, and `lstat()`, reporting:
- Inode number
- Hard link count
- File type (regular file, directory, symlink, FIFO, socket)
- File size and disk blocks allocated (`st_blocks * 512`)
- Detect any files with multiple hard links pointing to the same inode.

**Requirements**:
- Use `lstat()` instead of `stat()` so symlinks themselves are inspected rather than followed.
- Track visited inodes using `std::unordered_set<ino_t>` to detect circular symlink directories or duplicate hard links.
- Compare nominal file size (`st_size`) with actual physical block allocation (`st_blocks * 512`). Explain any sparse files discovered.

---

### Exercise 2: Zero-Copy File Transfer (`sendfile` / `copy_file_range`)
**Goal**: Implement a high-performance file copying utility that avoids copying file data through user space.

**Requirements**:
- On Linux, use `copy_file_range()` or `sendfile()`.
- On macOS, use `fcopyfile()` with `COPYFILE_DATA`.
- Benchmark the zero-copy implementation against a standard user-space buffer loop (`read()` into a 4 KB buffer then `write()`) for a 100 MB file.
- Explain the context switches and CPU cache pollution differences.

---

### Exercise 3: Simulating Atomic File Updates (Crash-Safe Writes)
**Goal**: Implement an atomic file write pattern used by production databases (e.g., SQLite, PostgreSQL WAL checkpoints).

**Requirements**:
1. Write new data to a temporary file in the same directory (`filename.tmp.XXXXXX`).
2. Call `fsync()` on the temporary file descriptor to flush data blocks and inode metadata to disk.
3. Use `rename()` to atomically replace the target file with the temporary file.
4. Call `fsync()` on the parent directory's file descriptor to ensure the directory entry update is durable.
5. Explain why writing directly to `filename` without atomic rename risks corrupting data if the power fails midway.
