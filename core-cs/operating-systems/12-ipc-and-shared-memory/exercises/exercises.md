# Hands-On Exercises: Inter-Process Communication & Shared Memory

---

### Exercise 1: Multi-Process Shared Memory Matrix Multiplier
**Goal**: Implement parallel matrix multiplication across $P$ separate processes (not threads!) using POSIX Shared Memory.

**Requirements**:
1. Allocate memory for input matrices $A$ ($N \times N$), $B$ ($N \times N$), and output matrix $C$ ($N \times N$) in a single `shm_open` / `mmap` region.
2. Fork $P$ worker processes, where each process is assigned a disjoint slice of rows in $C$ to compute.
3. Synchronize start and completion using POSIX semaphores or process-shared condition variables.
4. Benchmark speedup for $N = 1000$ with $P \in \{1, 2, 4, 8\}$.

---

### Exercise 2: File Descriptor Passing Daemon
**Goal**: Build a connection handler daemon that accepts TCP or socket connections and delegates them to worker processes using `SCM_RIGHTS`.

**Requirements**:
1. The supervisor process creates a Unix Domain Socket pair with a worker process.
2. The supervisor opens a file or accepts a connection.
3. The supervisor sends the open file descriptor to the worker using `sendmsg()` with `struct cmsghdr` containing `SCM_RIGHTS`.
4. The worker receives the new file descriptor using `recvmsg()`, reads the content, and verifies it matches the original file.

---

### Exercise 3: Fault-Tolerant Shared Memory with Robust Mutexes
**Goal**: Build a multi-process counter protected by a process-shared pthread mutex configured with `PTHREAD_MUTEX_ROBUST`.

**Requirements**:
1. Multiple processes increment a shared counter in an infinite loop.
2. Inject a random crash (`_exit(1)` or `kill(getpid(), SIGKILL)`) into a process while it holds the mutex.
3. Demonstrate that surviving processes intercept `EOWNERDEAD`, invoke `pthread_mutex_consistent()`, restore invariants, and continue operating without hanging.
