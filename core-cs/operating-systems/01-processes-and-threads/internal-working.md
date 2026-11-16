# Internal Working: Linux Kernel Process & Thread Architecture

## 1. The Unified Primitive: `task_struct` and `clone()`

Unlike some operating systems that maintain completely separate internal architectures for processes and threads, the **Linux Kernel treats both as schedulable entities called tasks**, represented by `struct task_struct` (defined in `<linux/sched.h>`).

Both `fork()` and `pthread_create()` are wrappers around the powerful `clone()` system call:

```c
int clone(int (*fn)(void *), void *stack, int flags, void *arg, ...);
```

The difference between a heavy process and a lightweight thread is defined simply by the `flags` passed to `clone()`:

| Flag Passed to `clone()` | Effect if Set (Thread - `pthread_create`) | Effect if Clear (Process - `fork`) |
| :--- | :--- | :--- |
| `CLONE_VM` | Shares same virtual memory address space (`mm_struct`). | Child gets a new, independent copy-on-write address space. |
| `CLONE_FILES` | Shares same open file descriptor table (`files_struct`). | Child gets a duplicated copy of open file descriptors. |
| `CLONE_FS` | Shares filesystem information (current directory `cwd`, umask). | Child gets independent filesystem state. |
| `CLONE_SIGHAND`| Shares signal handlers (`sighand_struct`). | Child gets independent copies of signal actions. |
| `CLONE_THREAD` | Child joins the caller's thread group (shares the same PID). | Child gets a new, unique PID. |

```
Inside the Linux Kernel:
Process with 2 Threads:
  task_struct (TID=1001, TGID/PID=1000) ──┐
                                          ├──► Shares mm_struct (Memory) & files_struct (FDs)
  task_struct (TID=1002, TGID/PID=1000) ──┘

Independent Child Process:
  task_struct (TID=1005, TGID/PID=1005) ─────► Dedicated mm_struct & dedicated files_struct
```
*Note*: `TGID` (Thread Group ID) is what user space observes as the `PID` returned by `getpid()`.

---

## 2. Zombie Processes & Reaping Mechanics

### Why Do Zombies Exist?
When a process calls `exit(int status)`:
1. The kernel releases all physical memory pages, closes all open file descriptors, and detaches shared libraries.
2. **However, the kernel cannot immediately delete the `task_struct` entry from the process table.**
3. Why? Because the parent process has the right to inspect how the child finished: Did it exit normally? What was its exit code? Was it killed by a signal (e.g. `SIGSEGV`)?
4. Until the parent calls `wait()` or `waitpid()`, the process remains in the **Zombie (`Z`)** state, occupying a PID slot.

### Danger of Zombie Leaks:
If a server program spawns thousands of child workers and neglects to call `waitpid()`, the system eventually exhausts the finite PID space (`/proc/sys/kernel/pid_max`, typically 32,768 or 4,194,304), preventing any new processes from spawning system-wide!

---

## 3. Orphan Adoption by PID 1

If a parent process terminates before its child process exits:
1. The child becomes an **Orphan**.
2. The kernel automatically re-parents the orphan process to **PID 1** (`systemd` on modern Linux, or `launchd` on macOS).
3. PID 1 executes a continuous background loop invoking `wait()` to automatically reap adopted children when they finish, preventing eternal zombies.

---

## 4. The Critical Trap: `fork()` in Multithreaded Programs

If a process has multiple threads running and one thread calls `fork()`:
1. **The child process inherits ONLY the thread that called `fork()`.** All other threads vanish instantly in the child!
2. **Deadlock Danger**: If another thread in the parent was holding a mutex (e.g., inside `malloc()`) at the exact instant `fork()` was invoked, that mutex remains marked as "locked" in the child's address space. But the thread that locked it no longer exists!
3. If the child attempts to allocate memory or acquire that lock, **it deadlocks permanently**.
4. **Golden Rule**: After calling `fork()` in a multithreaded application, the child process must ONLY invoke **async-signal-safe functions** or immediately call `execve()` to replace the memory space.
