# Theoretical Foundations: Inter-Process Communication (IPC)

Processes are isolated by the Memory Management Unit (MMU) and the OS kernel to guarantee memory safety. To collaborate, processes rely on IPC mechanisms provided by the operating system.

---

## 1. IPC Spectrum: Trade-Off Matrix

| IPC Mechanism | Kernel Involve / Copies | Data Boundary | Persistence | Throughput / Latency | Typical Use Case |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Anonymous Pipe** | 2 copies (User $\to$ Kernel $\to$ User) | Byte stream (no message boundaries) | Process lifetime | Medium / ~1–2 $\mu$s | Parent-child streaming (`cmd1 \| cmd2`) |
| **Named Pipe (FIFO)** | 2 copies | Byte stream | Filesystem entry (data in memory) | Medium / ~1–2 $\mu$s | Unrelated processes on same host |
| **Unix Domain Socket (`AF_UNIX`)** | 2 copies (optimized socket buffers) | Stream (`SOCK_STREAM`) or Datagram (`SOCK_DGRAM`) | Filesystem path or abstract namespace | High / ~1–3 $\mu$s | Local microservices, passing file descriptors |
| **POSIX Message Queue (`mq_*`)** | 2 copies | Structured discrete messages with priority | Kernel persistence | Medium / ~2–4 $\mu$s | Priority command dispatching |
| **POSIX Shared Memory (`shm_*`)** | **0 copies** (Direct CPU loads/stores) | Raw memory buffer (application defined) | Kernel / RAM filesystem (`/dev/shm`) | **Fastest** / **< 100 ns** | Ultra-low-latency HFT, video frames, DB buffer pools |

---

## 2. Pipes and FIFOs
- **Anonymous Pipes (`pipe(int pipefd[2])`)**: Unidirectional communication channels between related processes (sharing ancestry via `fork()`).
  - `pipefd[0]` is read end, `pipefd[1]` is write end.
  - Pipe buffer is typically 64 KB (on modern Linux). Writes up to `PIPE_BUF` (4096 bytes) are guaranteed atomic.
  - If a process reads from an empty pipe, it blocks until data is available. If writing to a full pipe, it blocks until buffer space is freed.
  - If all write ends are closed, `read()` returns 0 (`EOF`). If writing to a pipe with no active read ends, the kernel sends `SIGPIPE` to the writer.
- **Named Pipes (FIFOs, `mkfifo()` / `mknod()`)**: Has an entry in the filesystem tree, allowing arbitrary unrelated processes to open it by path name. Kernel buffering semantics remain identical to anonymous pipes.

---

## 3. Unix Domain Sockets (`AF_UNIX` / `AF_LOCAL`)
Unix Domain Sockets use the standard Berkeley sockets API (`socket()`, `bind()`, `listen()`, `accept()`, `connect()`), but all communication happens entirely within the OS kernel without entering the network protocol stack (no IP routing, checksumming, ARP, or NIC involvement).
- **Stream Sockets (`SOCK_STREAM`)**: Reliable, sequenced, two-way byte stream (like TCP, but without TCP headers or packet overhead).
- **Datagram Sockets (`SOCK_DGRAM`)**: Preserves message boundaries (like UDP), but 100% reliable and ordered (messages are never dropped or reordered).
- **Capabilities**:
  - File Descriptor Passing via `sendmsg()` with ancillary data (`cmsg_type = SCM_RIGHTS`).
  - Process credential inspection (`SO_PEERCRED` on Linux, `LOCAL_PEERCRED` on macOS) for secure authentication.

---

## 4. POSIX Shared Memory
Shared Memory bypasses the operating system kernel for data transfer. Both processes map the exact same physical RAM frames into their respective virtual address spaces.
- **Syscalls**:
  - `shm_open(const char* name, int oflag, mode_t mode)`: Creates or opens a POSIX shared memory object (backed by a tmpfs filesystem such as `/dev/shm` on Linux).
  - `ftruncate(int fd, off_t length)`: Sets the physical size of the shared memory region.
  - `mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)`: Maps the shared memory pages into the calling process's address space.
  - `munmap(void* addr, size_t length)`: Unmaps the region.
  - `shm_unlink(const char* name)`: Removes the shared memory object name from the system.
- **Zero-Copy Nature**: When Process A writes to the memory pointer, Process B reads the exact bytes immediately via CPU L1/L2/L3 cache coherence—zero system calls and zero memory copies.

---

## 5. Process-Shared Synchronization
Because the kernel is not involved in shared memory reads and writes, applications must coordinate access to prevent race conditions:
1. **POSIX Named Semaphores (`sem_open`, `sem_wait`, `sem_post`)**: Kernel-managed counters stored in system memory.
2. **Process-Shared Pthread Mutex & Condvar**:
   - Initialized with `pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)`.
   - Placed directly inside the shared memory structure.
   - Robust Mutexes (`pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)`): If a process crashes or is `SIGKILL`'d while holding the mutex, the next waiting process receives `EOWNERDEAD` instead of deadlocking indefinitely.
3. **Lock-Free Atomic Queues**: Using C++ `std::atomic` variables directly in shared memory for single-producer single-consumer (SPSC) circular ring buffers.
