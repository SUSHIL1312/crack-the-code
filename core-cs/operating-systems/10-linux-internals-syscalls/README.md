# Module 10: Linux Internals & System Calls

<!-- Chapter Navigation Header -->
| ⬅️ [Module 09: Concurrency & Parallelism](../09-concurrency-and-parallelism/README.md) | 🏠 [Operating Systems Hub](../README.md) | [Module 11: File Systems & I/O](../11-file-systems-and-io/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

The Linux kernel manages processes, hardware devices, storage, and networking through a unified system call interface and the Virtual Filesystem (VFS). This module explores low-level syscall mechanics, file descriptor tables, signal handling safety, `/proc` inspection, and non-blocking I/O multiplexing.

---

## 📑 Module Contents

1. [Theory & Systems Architecture](theory.md)
   * The Linux Kernel Architecture & Virtual Filesystem (VFS).
   * System Call Interface & Register ABI (x86-64 vs ARM64).
   * File Descriptors, File Tables, and Inodes.
   * I/O Multiplexing evolution: `select` $\to$ `poll` $\to$ `epoll`.
   * Unix Signals & Async-Signal Safety rules.
2. [Internal Working](internal-working.md)
   * Syscall dispatch table (`sys_call_table`) and `syscall` hardware instruction.
   * Level-Triggered (LT) vs Edge-Triggered (ET) in `epoll`.
   * The `/proc` pseudo-filesystem: How the kernel dynamically synthesizes process metrics.
   * Signal delivery stack frames and signal handler return via `sigreturn()`.
3. [Visual Explanations](visual-explanation.md)
   * The 3-tier File Descriptor relationship: Process FD table $\to$ Open File Description $\to$ Inode.
   * `epoll` Red-Black tree and Ready List architecture.
4. [Examples & Code](examples/)
   * `syscall_file_copy.cpp`: Production-grade file copy using raw POSIX syscalls (`open`, `read`, `write`, `close`), robustly handling `EINTR` and partial writes.
   * `signal_handling_demo.cpp`: Safe signal handling with `sigaction`, custom flags, and atomic flags.
   * `proc_parser_demo.cpp`: C++ parser inspecting process memory and status metrics via `/proc`.
5. [Exercises & Solutions](exercises/exercises.md)
   * Redirection and pipe mechanics using `dup2()`, designing self-pipe signal handling.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Critical FAANG questions: `select` vs `epoll`, Level vs Edge triggered, why `printf` in signal handlers causes deadlocks.
7. [Common Mistakes](common-mistakes.md)
   * Calling non-reentrant functions in signal handlers, file descriptor leaks, ignoring partial writes.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [Module 09: Concurrency & Parallelism](../09-concurrency-and-parallelism/README.md) | [Operating Systems Hub](../README.md) | **[Module 11: File Systems & I/O](../11-file-systems-and-io/README.md)** |
<!-- /Chapter Navigation Footer -->

