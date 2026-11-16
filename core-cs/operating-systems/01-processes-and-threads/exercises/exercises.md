# Exercises: Processes & Threads

Attempt each exercise independently before reading the reference solutions in [solutions.md](../solutions/solutions.md).

---

## 🟢 Beginner Level

### Exercise 1.1: Multi-Fork Process Count Prediction
Consider the following C++ program:
```cpp
#include <iostream>
#include <unistd.h>

int main() {
    fork();
    fork();
    fork();
    std::cout << "Process execution\n";
    return 0;
}
```
1. How many total times will `"Process execution"` be printed?
2. Draw the process tree showing parent-child relationships across the 3 `fork()` calls.
3. How many child processes were created in total?

### Exercise 1.2: Resource Sharing Inspection
Which of the following are shared between two threads of the same process?
- [ ] Heap memory allocated with `malloc()`
- [ ] Stack variables inside a thread worker function
- [ ] Global static variables
- [ ] File descriptors opened before thread creation
- [ ] Thread ID (`pthread_self()` / `std::this_thread::get_id()`)

---

## 🟡 Intermediate Level

### Exercise 2.1: Asynchronous Zombie Reaping via `SIGCHLD`
When a child process terminates, the kernel delivers a `SIGCHLD` signal to the parent.
1. Write a server-style parent process in C++ that spawns child worker processes in an infinite loop (e.g. once every second).
2. Install a `SIGCHLD` signal handler with `sigaction`.
3. In the signal handler, reap all terminated children using `waitpid(-1, &status, WNOHANG)` in a loop.
4. *Critical Interview Question*: Why MUST `waitpid` be called in a loop with `WNOHANG` inside a `SIGCHLD` handler instead of just calling `wait()` once?

---

## 🔴 Advanced Level

### Exercise 3.1: Reproducing the Multithreaded `fork()` Deadlock
1. Write a multithreaded program with two threads:
   - Thread A repeatedly locks a `std::mutex`, sleeps for 10 milliseconds, and unlocks it.
   - Thread B calls `fork()` in a loop.
2. In the child process created by Thread B, attempt to acquire the same `std::mutex`.
3. Observe how the child process deadlocks permanently when `fork()` captures the mutex while Thread A is holding it.
4. Implement a fix using POSIX `pthread_atfork(prepare, parent, child)` to guarantee lock safety across `fork()`.
