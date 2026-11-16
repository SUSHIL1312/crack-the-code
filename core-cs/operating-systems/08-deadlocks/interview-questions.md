# Interview Questions: Deadlocks

---

## Question 1: What are the Four Coffman Conditions? How do you prevent deadlocks in production code?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Systems Reliability & Concurrency
* **Classification**: Officially documented interview topic
* **Role Relevance**: All Software Engineers
* **Prerequisites**: Mutexes, Locks, Resource Allocation
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2–3 minute structured enumeration followed by practical software defenses.

### Model Answer
For a deadlock to occur, all **Four Coffman Conditions** must hold simultaneously:

1. **Mutual Exclusion**: Resources cannot be shared; only one thread can hold a resource at a time.
2. **Hold and Wait**: A thread holds at least one resource while waiting to acquire another.
3. **No Preemption**: Resources cannot be taken away by force; only voluntarily released.
4. **Circular Wait**: A closed loop of threads exists where $T_1$ waits for $T_2$, $T_2$ waits for $T_3$, ..., and $T_n$ waits for $T_1$.

### Practical Software Defenses:
In application engineering, **breaking Circular Wait** is the most practical and widely adopted strategy:
- **Global Lock Hierarchy (Lock Ordering)**: Enforce a strict total order on all locks (e.g. acquire Lock A before Lock B, or sort mutexes by memory address). If all threads acquire locks in the exact same order, a cycle in the dependency graph is mathematically impossible.
- **Simultaneous Multi-Lock Acquisition**: Use `std::scoped_lock(m1, m2)` in C++17, which internally uses a deadlock-avoidance algorithm (`std::lock`) using non-blocking `try_lock()` and backoff.
- **Lock Timeouts**: Use `std::timed_mutex` with `try_lock_for()`. If the lock cannot be acquired within 100 ms, release held locks, back off, and retry (breaking Hold and Wait).

---

## Question 2: What is the difference between an Unsafe State and a Deadlocked State?

* **Difficulty**: Easy
* **Importance**: High
* **Category**: Theoretical Operating Systems
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems & Backend Engineers
* **Prerequisites**: Banker's Algorithm
* **Estimated Preparation Time**: 10 minutes
* **Expected Answer Depth**: 1-minute crisp clarification with Venn diagram logic.

### Model Answer
- **A Deadlocked State** is an active failure: threads are *already blocked*, circular wait is actively present, and zero progress can ever be made without killing a process.
- **An Unsafe State** is NOT necessarily deadlocked yet: it is a state from which the operating system **cannot guarantee** that a deadlock will not occur in the future if all processes suddenly claim their maximum declared resources.
- **Relationship**:
  $$\text{Deadlocked States} \subset \text{Unsafe States}$$
  - A Safe state is guaranteed to be deadlock-free.
  - An Unsafe state *might* lead to a deadlock if processes request their maximum resources simultaneously, but if processes voluntarily terminate early without claiming maximums, the system might never deadlock.
  - The Banker's Algorithm prevents deadlocks by **never transitioning from a Safe state into an Unsafe state**.

---

## Question 3: What is Livelock? How does it differ from Deadlock?

* **Difficulty**: Medium
* **Importance**: High
* **Category**: Concurrency & Distributed Systems
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Engineers, Distributed Systems Engineers
* **Prerequisites**: Non-blocking algorithms, `try_lock`, Exponential backoff
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute explanation with an intuitive real-world analogy.

### Model Answer
Both Deadlock and Livelock represent permanent failure to make forward progress, but they differ fundamentally in **thread execution state and CPU utilization**:

- **Deadlock**:
  - Threads are in the **Blocked / Sleeping** state in the OS kernel.
  - CPU utilization is **0%**.
  - The threads are completely stationary, waiting on an event that never happens.
- **Livelock**:
  - Threads are in the **Running** state actively executing CPU instructions.
  - CPU utilization is **100%**.
  - The threads continuously change their internal state in response to each other, but the collective system makes **zero forward progress**.

### Concrete Example:
Two courteous threads try to avoid a deadlock using `try_lock()`:
```cpp
// Thread 1:
while (true) {
    lockA.lock();
    if (!lockB.try_lock()) {
        lockA.unlock(); // Be polite!
        continue;       // Retry
    }
    break;
}
// Thread 2:
while (true) {
    lockB.lock();
    if (!lockA.try_lock()) {
        lockB.unlock(); // Be polite!
        continue;       // Retry
    }
    break;
}
```
If both threads execute in lockstep, Thread 1 acquires A while Thread 2 acquires B; both fail `try_lock`, both unlock, both retry, and repeat forever!
- **Resolution**: Introduce randomized **Exponential Backoff with Jitter** before retrying.
