# Inter-Process Communication & Shared Memory: Top FAANG Interview Questions

### Q1: Google Chrome uses a multi-process architecture. How do the Browser Process, Renderer Processes, and GPU Process communicate efficiently and securely?
**Answer**:
- **Architecture**:
  - The **Browser Process** acts as the privileged broker.
  - **Renderer Processes** run in restrictive sandboxes (no direct network, no direct disk filesystem access).
  - The **GPU Process** handles hardware-accelerated rendering.
- **IPC Mechanisms Used**:
  1. **Mojo (IPC System)**: Chrome's modern IPC system built on top of platform-native primitives (Unix Domain Sockets on Linux/macOS, Named Pipes on Windows). Mojo passes structured, typed messages and handles message routing and object capability interfaces.
  2. **Shared Memory (`base::SharedMemory` / `base::UnsafeSharedMemoryRegion`)**:
     - Large assets such as rendered bitmaps, compositor frames, and decoded video buffers are placed into shared memory.
     - The Renderer writes the rendered raster tiles into shared memory, and passes a handle (file descriptor via Mojo `SCM_RIGHTS`) to the GPU or Browser process.
     - Zero-copy: 4K/60fps video frames do not traverse socket buffers or kernel copies.
  3. **Capability Brokering via File Descriptor Passing**: When a sandboxed renderer needs to load a local file chosen by the user in an open file dialog, the Browser process opens the file and transmits the open file descriptor across the Unix Domain Socket using `SCM_RIGHTS`. The renderer can read that single file without ever having general filesystem access.

---

### Q2: Compare Unix Domain Sockets vs TCP Loopback (`127.0.0.1`). Why do local microservices and databases (Redis, PostgreSQL) prefer UDS?
**Answer**:
1. **No Network Stack Overhead**:
   - TCP loopback passes through the IP routing layer, calculates TCP checksums, maintains TCP sequence numbers, handles window scaling, and constructs packet headers.
   - Unix Domain Sockets (`AF_UNIX`) operate purely as memory-to-memory queue transfers within the kernel (`sk_buff` ring buffer).
2. **Throughput and Latency**:
   - Unix Domain Sockets typically achieve **2x higher throughput** and **50% lower latency** than loopback TCP.
3. **Security and Permissions**:
   - Unix Domain Sockets are represented as filesystem nodes (`/var/run/redis.sock`), allowing standard POSIX permissions (`chmod`, `chown`, ACLs) to restrict access.
   - UDS supports peer credential checking (`SO_PEERCRED`), allowing a daemon to reliably know the UID and PID of the client process.
4. **Extra Capabilities**: UDS supports passing open file descriptors (`SCM_RIGHTS`), which is impossible over TCP.

---

### Q3: What is the risk of using a mutex in shared memory across processes, and how does `PTHREAD_MUTEX_ROBUST` solve it?
**Answer**:
- **The Risk (Deadlock by Death)**:
  - When Process A acquires a shared mutex (`pthread_mutex_lock()`) and crashes (e.g. segmentation fault, `abort()`, or `SIGKILL`), the mutex remains locked in shared memory.
  - Unlike in single-process multithreading where an unhandled exception terminates the entire process, in multi-process systems surviving processes continue running.
  - Any subsequent process attempting to acquire the mutex blocks forever in the kernel, causing permanent system freeze.
- **The Robust Mutex Solution**:
  - Configure mutex attributes:
    ```cpp
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    ```
  - When a process dies holding the mutex, the kernel detects it via the process's `robust_list`, unlocks the kernel futex, and sets the `FUTEX_OWNER_DIED` bit.
  - The next waiting process receives the return value `EOWNERDEAD` from `pthread_mutex_lock()`.
  - The acquiring process can inspect state consistency, fix any corrupted data, and call `pthread_mutex_consistent(&mutex)` before releasing it with `pthread_mutex_unlock()`.

---

### Q4: Why is a lock-free Single-Producer Single-Consumer (SPSC) ring buffer in shared memory faster than a mutex-protected queue?
**Answer**:
1. **Zero System Calls in the Fast Path**: Atomics (`std::memory_order_release` and `acquire`) map to simple CPU load/store instructions (and store barriers on ARM/TSO memory architectures). Mutexes may fall back to `futex()` system calls when contested, causing context switches (~1,000 to 2,000 ns).
2. **No Cache Contention (False Sharing)**: By placing the producer's `head` and the consumer's `tail` pointers on separate 64-byte cache lines (`alignas(64)`), each CPU core works on its own private cache line without invalidating the other core's cache.
3. **Zero Data Copies**: Data is written directly into shared memory slots and read in-place.
