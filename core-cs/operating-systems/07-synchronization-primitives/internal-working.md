# Internal Working: Linux Futex Mechanics & Spurious Wakeups

## 1. How a Modern Mutex Works: The Linux Futex

In early Unix implementations, acquiring a lock always required a system call into the kernel. Even if no other thread was competing for the lock, an uncontended application wasted ~100 nanoseconds on every single lock and unlock.

To solve this, Linux introduced the **Futex (Fast Userspace Mutex)** (Franke, Russell, Kirkwood, 2002):

```
                       Thread attempts to acquire Mutex
                                      │
                                      ▼
                      Atomic CAS (Compare-And-Swap) in User Space
                                      │
                     ┌────────────────┴────────────────┐
             Uncontended (CAS succeeds)       Contended (CAS fails)
                     │                                 │
                     ▼                                 ▼
           [FAST PATH: USER SPACE]            [SLOW PATH: KERNEL]
           Acquired in ~1-2 nanoseconds!      Invoke sys_futex(FUTEX_WAIT)
           ZERO system calls!                 Thread put to sleep on kernel wait queue
```

### The Futex Protocol:
1. **The Fast Path (Uncontended)**:
   - The mutex is represented by an integer in user-space memory (e.g. `0 = unlocked`, `1 = locked`, `2 = locked with waiters`).
   - Thread executes atomic instruction `lock cmpxchg` (0 $\to$ 1).
   - If successful, **the thread has acquired the lock entirely in User Space (Ring 3)** with zero kernel overhead!
2. **The Slow Path (Contended)**:
   - If the CAS fails because the lock is already held, the thread atomically changes the state to `2` (indicating waiters exist) and invokes the system call:
     ```c
     futex(uaddr, FUTEX_WAIT, 2, NULL, NULL, 0);
     ```
   - The kernel verifies that `*uaddr == 2`. If true, the kernel transitions the thread to **Blocked** state and enqueues it on a kernel hash table wait queue.
3. **Unlocking**:
   - The unlocking thread sets the integer from `1` to `0` via atomic store.
   - If the previous value was `2` (waiters exist), it invokes:
     ```c
     futex(uaddr, FUTEX_WAKE, 1, NULL, NULL, 0);
     ```
   - The kernel wakes up one sleeping thread from the wait queue.

---

## 2. Condition Variables & Spurious Wakeups

A thread waiting on a `std::condition_variable` can wake up **even if no thread ever signaled or broadcasted the condition variable**. This is called a **Spurious Wakeup**.

### Why Do Spurious Wakeups Happen?
1. **Kernel Signal Interrupts**: If a thread sleeping in `futex(FUTEX_WAIT)` receives a Unix signal (e.g. `SIGCHLD`, `SIGALRM`), the system call is interrupted and returns `EINTR`. The runtime library re-acquires the associated mutex and returns from `wait()`.
2. **Multi-Thread Race Between Wakeup and Lock Re-acquisition**:
   - Thread A signals the condition variable to wake Thread B.
   - Before Thread B can re-acquire the mutex, Thread C jumps in, acquires the mutex, consumes the item from the queue, and unlocks the mutex.
   - When Thread B finally acquires the mutex and resumes, the queue is empty again!
3. **Hardware & Multiprocessor Memory Ordering**: On some multi-core architectures, signal delivery cannot be perfectly serialized without costly global memory fences.

### The Mandatory Idiom: Always Use a `while` Loop!
```cpp
// ❌ DANGEROUS BUG: Using 'if'
std::unique_lock<std::mutex> lock(mtx);
if (queue.empty()) {
    cv.wait(lock); // If woken spuriously, queue is STILL EMPTY!
}
queue.pop(); // CRASH! Undefined behavior on empty container!

// ✅ CORRECT: Always use 'while' loop or predicate overload
while (queue.empty()) {
    cv.wait(lock); // Re-checks condition every time it wakes up
}
queue.pop(); // Guaranteed to have an item!

// Or equivalently in Modern C++:
cv.wait(lock, [&]() { return !queue.empty(); });
```
