# Common Mistakes & Traps: Linux Internals & System Calls

---

## 1. Forgetting `O_CLOEXEC` on File Descriptors

### The Vulnerability:
```cpp
// Opening a sensitive database file or security socket
int fd = open("/etc/ssl/private.key", O_RDONLY);
// ... later in the code:
if (fork() == 0) {
    execve("/bin/user_script", argv, envp); // ❌ LEAKS PRIVATE KEY FD TO CHILD!
}
```

### Why It Fails:
In POSIX, open file descriptors remain open across `execve()` by default! If your server program executes an untrusted script or child utility, that child inherits file descriptor `fd` and can read confidential data or hijack network sockets.

### The Fix:
Always use `O_CLOEXEC` when opening files or sockets:
```cpp
int fd = open("/etc/ssl/private.key", O_RDONLY | O_CLOEXEC); // ✅ Closed automatically on exec!
```

---

## 2. Assuming `write()` Writes All Requested Bytes

### The Bug:
```cpp
// ANTI-PATTERN: Blindly assuming write() completes in one call
const char* data = "Massive payload of 100,000 bytes...";
write(socket_fd, data, 100000); // ❌ Likely only writes 8 KB or 16 KB!
```

### Why It Fails:
`write()` returns the **actual number of bytes transferred**. If the socket or pipe buffer only has 16 KB of available space, `write()` will return `16384`. The remaining 83,616 bytes are silently dropped unless the application loops until the entire buffer has been transmitted!

---

## 3. Ignoring `EINTR` (Interrupted System Call)

### The Bug:
```cpp
// ANTI-PATTERN: Treating -1 as an unrecoverable failure
ssize_t n = read(fd, buf, size);
if (n < 0) {
    throw std::runtime_error("Read failed!"); // ❌ Crashes on harmless SIGCHLD!
}
```

### Why It Fails:
If a system call is blocked waiting for I/O and a signal arrives (e.g. `SIGCHLD` from a dead child), the kernel interrupts the system call and returns `-1` with `errno == EINTR`. This is NOT an error; it simply means the syscall was interrupted. The correct pattern is to check `if (errno == EINTR) continue;` and retry.
