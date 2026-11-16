# Visual Explanation: Context Switching & Kernel Stack Swapping

## 1. Mode Switch vs Full Context Switch

```
A. MODE SWITCH (System call execution, e.g. write()):
   User Space (Ring 3)              Kernel Space (Ring 0)
   ┌───────────────────────┐        ┌───────────────────────┐
   │ Task 1 running        │        │                       │
   │ write(fd, buf, len)   │──TRAP─►│ Kernel handles write  │
   │                       │        │ (Same task identity)  │
   │ Resumes Task 1        │◄──RET──│ Returns result        │
   └───────────────────────┘        └───────────────────────┘

B. CONTEXT SWITCH (Task 1 preempted, Task 2 scheduled):
   Task 1 (Ring 3)                  Kernel (Ring 0)                  Task 2 (Ring 3)
   ┌───────────────┐                ┌───────────────┐                ┌───────────────┐
   │ Task 1 running│                │               │                │               │
   │ [Timer Tick]  │──INTERRUPT────►│ Save Task 1   │                │               │
   │               │                │ Registers     │                │               │
   │               │                │               │                │               │
   │               │                │ Scheduler     │                │               │
   │               │                │ picks Task 2  │                │               │
   │               │                │               │                │               │
   │               │                │ Swap RSP &    │                │               │
   │               │                │ Restore Task 2│────RESUME─────►│ Task 2 runs   │
   └───────────────┘                └───────────────┘                └───────────────┘
```

---

## 2. Kernel Stack Pointer Swapping Mechanism

When switching from Task 1 to Task 2, the CPU stack pointer `RSP` is reassigned:

```
Task 1 Kernel Stack (Before Switch)              Task 2 Kernel Stack (Before Switch)
Address: 0xFFFF8800_1000                        Address: 0xFFFF8800_2000
┌─────────────────────────────────┐             ┌─────────────────────────────────┐
│ Saved Task 1 Registers:         │             │ Saved Task 2 Registers:         │
│ R15, R14, R13, R12, RBX, RBP    │             │ R15, R14, R13, R12, RBX, RBP    │
├─────────────────────────────────┤             ├─────────────────────────────────┤
│ Return Address (RIP of Task 1)  │             │ Return Address (RIP of Task 2)  │
└─────────────────────────────────┘             └─────────────────────────────────┘
                ▲                                               ▲
                │                                               │
       [RSP points here]                               [prev->thread.sp]
                │
                └─────────── SWITCH RSP REGISTER ───────────────┘
                                   │
                                   ▼
                   CPU RSP now points to Task 2 Stack!
                   'ret' instruction pops Task 2's RIP!
```
