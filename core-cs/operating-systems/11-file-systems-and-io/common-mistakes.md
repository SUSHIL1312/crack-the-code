# Common Mistakes in File Systems and I/O

### 1. Assuming `write()` Guarantees Durability on Disk
- **Mistake**: Writing data to a file descriptor and closing it, assuming it survived a power failure or OS crash.
- **Reality**: `write()` merely copies bytes into the kernel's Page Cache in volatile RAM. Even `close(fd)` does not flush to disk; it only frees the process's file descriptor reference.
- **Fix**: Explicitly call `fsync(fd)` or `fdatasync(fd)` before closing if the data must be durable across crashes.

---

### 2. Failing to Fsync the Parent Directory on Creation or Rename
- **Mistake**: Writing a new file or renaming a file atomically, calling `fsync(file_fd)`, and assuming the file is safe.
- **Reality**: The file content is synced to the inode, but the **directory entry** that points the directory to that inode lives in the parent directory's data blocks. If the machine crashes before the parent directory is written to disk, the inode exists as an orphan on disk, but the filename does not exist in the directory!
- **Fix**: Open the parent directory (`open(parent_dir, O_RDONLY | O_DIRECTORY)`) and call `fsync(dir_fd)` after renaming or creating new critical files.

---

### 3. Misunderstanding `O_APPEND` in Multi-Process Logging
- **Mistake**: Calling `lseek(fd, 0, SEEK_END)` followed by `write(fd, buf, len)` when multiple processes write to the same log file.
- **Reality**: The `lseek()` and subsequent `write()` are two separate syscalls. A race condition occurs if process B writes between process A's seek and write, overwriting process B's data!
- **Fix**: Open the file with `O_APPEND`. In POSIX-compliant kernels, `O_APPEND` guarantees that seeking to the end of the file and writing occurs atomically inside the kernel lock.

---

### 4. Reading Line-by-Line with Unbuffered `read()` Syscalls
- **Mistake**: Invoking `read(fd, &ch, 1)` in a loop to read text files character-by-character.
- **Reality**: Each `read()` syscall incurs a full user-to-kernel context switch (~1,000 to 2,000 CPU cycles). Reading a 10 MB file character-by-character causes 10,000,000 syscalls, slowing execution by orders of magnitude.
- **Fix**: Use buffered I/O (`std::ifstream`, C standard library `fread` / `fgets` with default 8 KB buffer), or allocate an explicit 64 KB user-space buffer and parse lines in memory.
