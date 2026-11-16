# Theory: Processes, Threads, and Concurrency Models

## 1. Program vs Process vs Thread

- **Program**: A passive entity stored on disk—an executable binary file (e.g. ELF or Mach-O) containing machine instructions and static data.
- **Process**: An active instance of an executing program. It encompasses:
  - An isolated virtual address space (`.text`, `.data`, heap, stack).
  - Dedicated OS resources (file descriptor table, network sockets, signal handlers).
  - Security credentials (UID, GID).
- **Thread**: The smallest unit of execution scheduled by the OS kernel. A thread exists *within* the context of a process. Multiple threads in the same process share the same virtual address space and file descriptors, but each possesses its own private execution state.

### Real-World Analogy
Think of a software engineering company:
- The **Program** is the architectural blueprint in a binder on a shelf.
- The **Process** is the office building leased for a specific project, equipped with conference rooms, coffee machines, and security keycards (address space & file descriptors).
- The **Threads** are the individual software engineers working inside that building. They share the conference rooms and whiteboard markers (heap & globals), but each engineer has their own notebook and train of thought (private stack and CPU registers).

---

## 2. Process States & Lifecycle

An operating system kernel manages processes through distinct state transitions:

```
                      [Admitted]
                          │
                          ▼
        ┌──────────► [ READY ] ◄──────────┐
        │                 │               │
  [I/O Done /             │ [Scheduler    │ [Timer Interrupt /
   Signal]                │  Dispatch]    │  Preemption]
        │                 ▼               │
    [ BLOCKED ] ◄── [ RUNNING ] ──────────┘
 (Waiting on I/O,         │
  Lock, Sleep)            │ [Exit / Terminate]
                          ▼
                     [ ZOMBIE ]
                          │ [Parent calls waitpid()]
                          ▼
                    [TERMINATED]
```

1. **New / Admitted**: The OS is allocating memory structures (PCB) for the process.
2. **Ready**: The process is loaded in memory and waiting for CPU time from the scheduler.
3. **Running**: Instructions are actively executing on a CPU core.
4. **Blocked / Waiting**: The process cannot run because it is waiting for an external event (disk I/O, socket data, mutex acquisition, or sleep timer).
5. **Zombie (Defunct)**: The process has finished execution, but its exit status code is retained in the kernel process table until its parent retrieves it via `waitpid()`.
6. **Terminated**: All kernel resources and PID table entries are completely reclaimed.

---

## 3. PCB vs TCB: What is Shared vs Private?

The kernel tracks each process via a **Process Control Block (PCB)** (`task_struct` in Linux). Inside each process, each thread has a **Thread Control Block (TCB)**.

| Resource / State | Process-Level (Shared by all Threads) | Thread-Private (Exclusive to each Thread) |
| :--- | :---: | :---: |
| **Virtual Address Space (Heap, BSS, Data, Text)** | ✅ Shared | ❌ |
| **Open File Descriptors (`0: stdin`, `1: stdout`, sockets)**| ✅ Shared | ❌ |
| **Current Working Directory & Root (`cwd`)** | ✅ Shared | ❌ |
| **Signal Handlers (`sigaction`)** | ✅ Shared | ❌ |
| **Process ID (`PID`) & Parent PID (`PPID`)** | ✅ Shared | ❌ |
| **Program Counter (Instruction Pointer `PC`/`RIP`)** | ❌ | ✅ Private |
| **CPU Register Set (General Purpose, Status Flags)** | ❌ | ✅ Private |
| **Stack Memory (Stack frame, local variables)** | ❌ | ✅ Private (each thread has its own stack) |
| **Thread-Local Storage (`thread_local` / TLS)** | ❌ | ✅ Private |
| **Pending Signal Mask & Thread ID (`TID`)** | ❌ | ✅ Private |

---

## 4. Multithreading Models

How are user-level threads mapped to operating system kernel schedulable units?

```
    1:1 Model (Modern Linux/POSIX)          N:1 Model (Green Threads)           M:N Model (Go Goroutines)
 User Threads      Kernel Threads       User Threads       Kernel Thread    User Threads       Kernel Threads
     [T1] ───────► [K1]                      [T1] ──┐                            [T1] ──┐          [K1]
     [T2] ───────► [K2]                      [T2] ──┼──► [K1]                    [T2] ──┼──► ───►  [K2]
     [T3] ───────► [K3]                      [T3] ──┘                            [T3] ──┘          [K3]
```

1. **1:1 Model (Kernel-level Threading - `pthreads`, `std::thread`, Java, C#)**:
   - Every user thread maps directly to one kernel schedulable entity (`task_struct`).
   - *Pros*: True multicore hardware parallelism; a blocking I/O call in one thread does not block other threads.
   - *Cons*: Thread creation and context switching require kernel mode transitions.
2. **N:1 Model (User-level Threading - Old Python greenlets, Early Java)**:
   - Multiple user threads run on a single kernel thread managed by a user-space runtime.
   - *Pros*: Ultra-fast switching (no syscalls).
   - *Cons*: Cannot utilize multiple CPU cores; one blocking system call blocks all user threads.
3. **M:N Model (Hybrid / Two-Level - Go Goroutines, Erlang Processes)**:
   - $M$ lightweight user routines multiplexed over $N$ kernel worker threads.
   - *Pros*: Combines fast user-space switching with multi-core utilization.
