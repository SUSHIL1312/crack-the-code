# Solutions: Processes & Threads

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Multi-Fork Process Count Prediction
1. **Total prints**: $2^3 = 8$ times.
2. **Process Tree**:
   ```
                        P0 (Root)
                    /       |       \
               P1 (Fork 1)  |        \
              /    \        |         \
           P2(F2)  P3(F3)  P4(Fork 2)  P6(Fork 3)
           /                 \
         P5(F3)              P7(F3)
   ```
   - Initially: 1 process ($P_0$).
   - After Fork 1: 2 processes ($P_0, P_1$).
   - After Fork 2: 4 processes ($P_0, P_1, P_2, P_4$).
   - After Fork 3: 8 processes ($P_0, \dots, P_7$).
3. **Total child processes created**: $8 - 1 = 7$ new child processes.

### Exercise 1.2: Resource Sharing Inspection
- [x] **Heap memory allocated with `malloc()`** (Shared)
- [ ] Stack variables inside a thread worker function (Thread-private)
- [x] **Global static variables** (Shared)
- [x] **File descriptors opened before thread creation** (Shared)
- [ ] Thread ID (`pthread_self()`) (Thread-private)

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Asynchronous Zombie Reaping via `SIGCHLD`

```cpp
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>

void sigchld_handler(int signo) {
    (void)signo;
    // CRITICAL: Must use a while loop with WNOHANG!
    // Unix signals are NOT queued. If 5 children exit simultaneously,
    // only ONE SIGCHLD signal may be delivered to the parent.
    // The while loop ensures ALL pending dead children are reaped.
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Successfully reaped child with PID 'pid'
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, nullptr);

    // Parent can now spawn children without blocking or leaking zombies!
    for (int i = 0; i < 5; ++i) {
        if (fork() == 0) {
            _exit(0); // Child exits immediately
        }
    }
    sleep(1); // Give signal handler time to reap
    return 0;
}
```

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Reproducing & Fixing Multithreaded `fork()` Deadlock

```cpp
#include <pthread.h>
#include <unistd.h>
#include <iostream>

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// Handlers registered via pthread_atfork:
void prepare_fork() {
    // 1. Acquire mutex BEFORE forking to ensure no other thread holds it
    pthread_mutex_lock(&g_mutex);
}

void parent_post_fork() {
    // 2. Unlock in parent after fork completes
    pthread_mutex_unlock(&g_mutex);
}

void child_post_fork() {
    // 3. Unlock in child so child starts with an unlocked, clean mutex
    pthread_mutex_unlock(&g_mutex);
}

void setup_safe_forking() {
    // Register the three lifecycle handlers
    pthread_atfork(prepare_fork, parent_post_fork, child_post_fork);
}
```
`pthread_atfork()` guarantees that the parent thread acquiring the lock stops all other threads from modifying guarded invariants during the fork split, ensuring the child starts with a consistent, unlocked state.
