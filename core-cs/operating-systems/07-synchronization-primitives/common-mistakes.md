# Common Mistakes & Traps: Synchronization Primitives

---

## 1. The "Hurry-Up and Wait" Antipattern with Condition Variables

### The Anti-Pattern:
```cpp
void producer() {
    std::unique_lock<std::mutex> lock(mtx);
    queue.push(data);
    cv.notify_one(); // ❌ Notifying while STILL HOLDING the mutex!
    // lock destructs here
}
```

### What Happens Internally:
1. `cv.notify_one()` wakes up the sleeping consumer thread.
2. The consumer thread immediately attempts to re-acquire `mtx`.
3. But `mtx` is **still held by the producer**!
4. The consumer fails to acquire the mutex and is immediately put back to sleep by the kernel.
5. A few microseconds later, the producer exits scope and unlocks `mtx`. The kernel has to wake the consumer thread a second time!
6. This causes unnecessary context switches and thread ping-ponging.
- **The Fix**: Whenever possible, unlock the mutex *before* signaling:
  ```cpp
  void producer() {
      {
          std::lock_guard<std::mutex> lock(mtx);
          queue.push(data);
      } // Mutex unlocked here
      cv.notify_one(); // ✅ Consumer wakes up and immediately grabs the free lock!
  }
  ```

---

## 2. Holding Locks Across Long-Running I/O Operations

### The Bug:
```cpp
void handle_request(int client_fd) {
    std::lock_guard<std::mutex> lock(global_mutex); // ❌ Giant coarse-grained lock!
    std::string response = read_from_slow_database(); // Takes 50 milliseconds!
    write(client_fd, response.data(), response.size()); // Takes network I/O time!
}
```

### The Impact:
Every other thread in the server is completely stalled for tens of milliseconds waiting for this single thread to complete its database and network I/O.
- **Rule**: Never hold a lock across blocking system calls, disk writes, or network sockets. Only hold the lock for the minimum lines necessary to read or write the in-memory data structure.
