# Visual Explanation: Linux System Calls & `epoll` Architecture

## 1. System Call Dispatch Architecture

```
User Program
│
├── Calls write(1, buf, len)
│   │
│   ▼
│   Glibc Wrapper (sysdeps/unix/sysv/linux/x86_64/write.c)
│   ├── Loads SYS_write into %rax
│   ├── Executes 'syscall'
│   │
════╪═══════════════════════════════════════════════════════════════════ PRIVILEGE BOUNDARY
│   ▼
Linux Kernel (Ring 0)
├── entry_SYSCALL_64:
│   ├── Saves CPU registers into pt_regs on Kernel Stack
│   ├── Validates syscall number in %rax
│   └── Calls sys_call_table[%rax]  ───► sys_write(fd, buf, count)
│                                             │
│                                             ▼
│                                        VFS: vfs_write()
│                                             │
│                                             ▼
│                                        Device Driver / Socket Write
│                                             │
│   ┌─────────────────────────────────────────┘
│   ▼
└── Places return value into %rax
    Executes 'sysret'
════╪═══════════════════════════════════════════════════════════════════ PRIVILEGE BOUNDARY
│   ▼
Resumes User Program instruction execution!
```

---

## 2. `epoll` Red-Black Tree vs Ready List

```
                          epoll Instance (epfd)
                        ┌───────────────────────┐
                        │ Red-Black Tree (rbr)  │
                        │                       │
                        │       [Socket 5]      │
                        │       /        \      │
                        │  [Socket 2]  [Socket 8]
                        └──────────┬────────────┘
                                   │
                  Data arrives on Socket 8!
                  ep_poll_callback() executes
                                   │
                                   ▼
                        ┌───────────────────────┐
                        │   Ready List (rdllist)│
                        │   [Socket 8] ──► NULL │
                        └──────────┬────────────┘
                                   │
                        epoll_wait() returns immediately with Socket 8!
```
