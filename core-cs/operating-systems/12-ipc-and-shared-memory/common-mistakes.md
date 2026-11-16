# Common Mistakes in Inter-Process Communication & Shared Memory

### 1. Leaking Shared Memory Objects in `/dev/shm`
- **Mistake**: Calling `shm_open()` to allocate 1 GB of memory, and terminating the process without calling `shm_unlink()`.
- **Reality**: POSIX shared memory objects have **kernel persistence**, not process persistence. They remain allocated in RAM (backed by `tmpfs` at `/dev/shm`) until explicitly unlinked or until system reboot. Repeated runs will exhaust host RAM!
- **Fix**: Register an `atexit()` handler or RAII guard that calls `shm_unlink()`, or manually clean up via `rm /dev/shm/<name>`.

---

### 2. Passing Raw Pointers in Shared Memory Structures
- **Mistake**: Defining a struct with standard pointers or containers (e.g. `struct Node { int data; Node* next; }` or `std::string`, `std::vector`) in shared memory.
- **Reality**:
  1. Virtual address spaces of Process A and Process B are randomized (ASLR). Process A might map the shared region at `0x7000'0000`, while Process B maps it at `0x5000'0000`. A raw pointer `next = 0x7000'0040` written by Process A is an invalid memory address in Process B, leading to `SIGSEGV`.
  2. Standard STL containers (`std::vector`, `std::string`) allocate their dynamic buffers on the process's **private heap**, not inside the shared memory region!
- **Fix**: Use **relative offsets** (e.g. `uint32_t offset_from_base`) or IPC-aware allocators (`boost::interprocess::offset_ptr` / fixed-size arrays).

---

### 3. Pipe Deadlocks from Buffer Saturation
- **Mistake**: Two processes communicate using two pipes (one for request, one for response). Process A writes a 10 MB payload before reading the response, while Process B reads a few bytes, tries to write a response, and blocks.
- **Reality**: The default Linux pipe capacity is only 64 KB. Process A blocks writing byte 65,537 because the pipe is full. Process B blocks writing its response because Process A is not reading yet. Both processes wait forever in kernel state `wait_queue_head_t` (Deadlock).
- **Fix**: Use non-blocking I/O (`O_NONBLOCK`), I/O multiplexing (`poll` / `epoll`), or spawn dedicated reader and writer threads.

---

### 4. False Sharing in Shared Memory Atomics
- **Mistake**: Placing `std::atomic<uint64_t> producer_head` and `std::atomic<uint64_t> consumer_tail` directly adjacent to each other in shared memory.
- **Reality**: Both variables share the same 64-byte L1 cache line. Every time the producer updates `head`, the consumer's CPU cache line is invalidated via MESI cache protocol, cutting throughput by 4x to 10x.
- **Fix**: Use `alignas(64)` (or `alignas(std::hardware_destructive_interference_size)`) to force each variable onto its own dedicated cache line.
