# Theory: Synchronization Primitives & Thread Safety

## 1. The Critical Section Problem & Race Conditions

A **Race Condition** occurs when two or more concurrent threads access shared memory simultaneously, at least one access is a write, and the accesses are not synchronized.

A **Critical Section** is the block of code that accesses the shared resource. Any valid synchronization solution must satisfy three non-negotiable criteria (Dijkstra's Criteria):
1. **Mutual Exclusion**: If thread $T_1$ is executing inside the critical section, no other thread may enter.
2. **Progress**: If no thread is in the critical section and some threads want to enter, only those threads can participate in the decision, and selection cannot be postponed indefinitely.
3. **Bounded Waiting**: There must be a bound on the number of times other threads are allowed to enter their critical sections after a thread has requested entry (guarantees freedom from starvation).

---

## 2. Synchronization Primitives Comparison Matrix

| Primitive | Mechanism | Ownership? | Blocking vs Spinning | CPU Cost | Best Use Case |
| :--- | :--- | :---: | :--- | :--- | :--- |
| **Mutex** (`std::mutex`) | Mutual exclusion | ✅ **Yes** (Only locking thread may unlock) | **Blocks** via OS kernel (Futex) | Zero CPU while waiting; ~1–2 $\mu$s context switch to sleep/wake | General-purpose critical sections with moderate/long wait times |
| **Spinlock** (`atomic_flag`) | Busy-wait loop | ❌ No | **Spins** (Burns 100% CPU on core) | High CPU burn; sub-microsecond acquisition if wait is tiny | Very short critical sections (< 500 ns) or OS interrupt handlers |
| **Counting Semaphore** | Integer counter | ❌ **No** (Any thread can `release()`) | **Blocks** when counter == 0 | Low; uses wait queue | Resource pools (e.g. limiting DB connections to 10), thread signaling |
| **Condition Variable** | Wait queue linked with mutex | ❌ No | **Blocks** atomically releasing mutex | Low; wakes on `notify_one()` / `notify_all()` | Producer-consumer queues, event notification |
| **Read-Write Lock** (`shared_mutex`)| Shared / Exclusive access | ✅ Exclusive mode | **Blocks** writers if readers active | Moderate overhead | Read-heavy workloads (> 90% reads, e.g. in-memory routing tables) |
| **Barrier** (`std::barrier`)| Rendezvous sync point | ❌ No | **Blocks** until $N$ threads arrive | Low | Phase-based parallel algorithms (e.g. matrix multiplication phases) |

---

## 3. Mutex vs Semaphore: The Most Critical Distinction

In engineering interviews, candidates frequently confuse Mutexes and Binary Semaphores:

- **A Mutex is a Locking Mechanism with Ownership**:
  - The thread that calls `mutex.lock()` **MUST** be the exact thread that calls `mutex.unlock()`.
  - If Thread A locks a mutex and Thread B attempts to unlock it, the behavior is undefined.
  - Supports priority inheritance to prevent **Priority Inversion**.
- **A Semaphore is a Signaling Mechanism WITHOUT Ownership**:
  - A semaphore maintains an integer token count.
  - `acquire()` decrements the count (blocking if 0); `release()` increments the count.
  - **Thread A can wait on a semaphore, and Thread B can signal (release) it!**
  - Used for inter-thread coordination and producer-consumer synchronization.
