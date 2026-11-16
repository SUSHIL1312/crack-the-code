# Project 05: Multi-Process Producer-Consumer with POSIX Shared Memory and Semaphores

A high-performance inter-process producer-consumer queue operating across independent process boundaries using POSIX Shared Memory (`mmap`) and POSIX Semaphores.

---

## Architectural Highlights
- **Zero Kernel Data Copies**: Data items are written directly to physical RAM pages mapped into both the producer and consumer address spaces (`MAP_SHARED`).
- **POSIX Semaphore Synchronization**:
  - `empty_slots_sem`: Initialized to `BUFFER_CAPACITY`. Producer waits on this before writing.
  - `filled_slots_sem`: Initialized to `0`. Consumer waits on this before reading.
  - `mutex_sem`: Protects ring buffer indices (`head`/`tail`) for multi-producer/multi-consumer safety.
- **Portability**: Uses POSIX `sem_open` with unique namespace keys or anonymous shared memory semaphores, ensuring compatibility across macOS and Linux.
- **Data Integrity**: Every message carries a monotonic sequence number and payload hash verified by the consumer.

---

## Build and Run
```bash
make
./shm_ipc_test
```
