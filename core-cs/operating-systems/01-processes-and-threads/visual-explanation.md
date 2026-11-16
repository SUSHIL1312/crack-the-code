# Visual Explanation: Processes, Threads & Address Space Isolation

## 1. Process Isolation vs Multithreaded Sharing

### Two Independent Processes (via `fork()`)
Each process operates inside its own private virtual address space. Modifications in Process B do NOT affect Process A.

```
       Process A (PID: 1001)                      Process B (PID: 1002)
┌─────────────────────────────────┐        ┌─────────────────────────────────┐
│ Stack (Local variables)         │        │ Stack (Local variables)         │
├─────────────────────────────────┤        ├─────────────────────────────────┤
│ Heap (malloc / new)             │        │ Heap (malloc / new)             │
│ [Address 0x5000: value = 42]    │        │ [Address 0x5000: value = 99]    │
├─────────────────────────────────┤        ├─────────────────────────────────┤
│ Data / BSS (Global variables)   │        │ Data / BSS (Global variables)   │
├─────────────────────────────────┤        ├─────────────────────────────────┤
│ Text (Code instructions)        │        │ Text (Code instructions)        │
└─────────────────────────────────┘        └─────────────────────────────────┘
     [Hardware MMU Page Tables]                 [Hardware MMU Page Tables]
                 │                                          │
                 ▼                                          ▼
   Physical RAM Frame 0x00A100                Physical RAM Frame 0x00F800
```

---

### One Process with Two Threads (via `std::thread`)
Both threads share the same Heap, Data, BSS, and Text segments. Each thread only has its own Stack and Register state.

```
                   Single Process Address Space (PID: 2000)
┌────────────────────────────────────────────────────────────────────────┐
│  Thread 1 Stack (TID: 2001)           │  Thread 2 Stack (TID: 2002)    │
│  [Local vars, return address]         │  [Local vars, return address]  │
├───────────────────────────────────────┴────────────────────────────────┤
│                                                                        │
│  SHARED HEAP (malloc / new)                                            │
│  Both threads can read/write address 0x5000 directly!                  │
│  ⚠️ (Requires std::mutex to prevent Race Conditions)                    │
│                                                                        │
├────────────────────────────────────────────────────────────────────────┤
│  SHARED DATA & BSS (Global and static variables)                       │
├────────────────────────────────────────────────────────────────────────┤
│  SHARED TEXT (Machine code instructions)                               │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. The Zombie Process Timeline

```mermaid
sequenceDiagram
    autonumber
    participant Parent as Parent Process
    participant Kernel as Linux Kernel (Process Table)
    participant Child as Child Process

    Parent->>Kernel: fork()
    Kernel->>Child: Spawn child process (PID 5001)
    Child->>Child: Perform workload
    Child->>Kernel: exit(0)
    Note over Child,Kernel: Child memory freed; task_struct preserved
    Kernel-->>Parent: SIGCHLD signal sent
    Note over Kernel: PID 5001 is now a ZOMBIE [defunct]
    Parent->>Parent: Busy working, hasn't called waitpid() yet...
    Parent->>Kernel: waitpid(5001, &status, 0)
    Kernel-->>Parent: Return exit status code
    Kernel->>Kernel: Reclaim task_struct & release PID 5001
    Note over Kernel: Process 5001 completely TERMINATED
```
