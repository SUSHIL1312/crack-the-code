# Interview Questions: Computer Fundamentals & Program Execution

---

## Question 1: What happens internally when you execute a program from the shell?

* **Difficulty**: Medium
* **Importance**: Critical
* **Category**: Systems Architecture & Kernel Internals
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems Software Engineer, Backend Infrastructure, SRE
* **Prerequisites**: Shell basics, system call interfaces, ELF structure
* **Estimated Preparation Time**: 20 minutes
* **Expected Answer Depth**: 3–5 minute structured walkthrough from user keystroke to `main()` execution.

### Model Answer
Executing a program (e.g. `./app`) involves coordination between the terminal shell, the Linux kernel, and the dynamic linker:

1. **User Request & Process Fork**:
   - The shell parses the command string and issues a `fork()` system call, creating a duplicate child process.
2. **Binary Invocation (`execve`)**:
   - The child process invokes `execve("./app", argv, envp)`.
   - The kernel validates permissions, clears the child's old address space, and reads the binary's ELF header.
3. **Memory Mapping & Layout**:
   - The kernel maps the executable's segments into virtual memory (`.text`, `.rodata` as read-only/execute; `.data`, `.bss` as read-write).
   - An initial stack and heap are allocated via virtual memory regions (VMA).
   - Command-line arguments (`argv`) and environment variables (`envp`) are pushed onto the newly allocated user stack.
4. **Interpreter / Dynamic Linker**:
   - If the binary is dynamically linked, the kernel maps the dynamic linker (typically `/lib64/ld-linux-x86-64.so.2`) into memory and sets the initial program counter (PC) to the linker's entry point.
   - The linker loads required shared libraries (`.so`), resolves undefined symbols, and applies runtime relocations.
5. **Runtime Initialization**:
   - Execution jumps to the binary's entry point (`_start` in glibc).
   - `_start` invokes `__libc_start_main`, which initializes the C runtime, sets up thread-local storage (TLS), and executes constructors in `.init_array`.
6. **Execution of `main()`**:
   - `__libc_start_main` calls `main(argc, argv, envp)`. When `main` returns, `exit()` is called, triggering destructors in `.fini_array` and terminating the process via `exit_group()` system call.

### Common Pitfalls / Candidate Traps
* **Trap 1**: Claiming `main()` is the very first function executed. In reality, `_start` executes first, followed by `__libc_start_main` and global static constructors.
* **Trap 2**: Forgetting the role of `fork()` vs `execve()`. `fork()` creates the process; `execve()` replaces the memory image.

### Follow-up Questions
1. *What happens to open file descriptors across `execve()`?* (They remain open unless marked with `FD_CLOEXEC` / `O_CLOEXEC`).
2. *How does Address Space Layout Randomization (ASLR) alter this sequence?* (The kernel applies a random delta offset to segment bases to prevent predictable memory exploits).

---

## Question 2: Difference between a System Call and a Standard Library Function Call?

* **Difficulty**: Easy
* **Importance**: Critical
* **Category**: OS Interface & CPU Architecture
* **Classification**: Common industry interview theme
* **Role Relevance**: All Software Engineers
* **Prerequisites**: CPU privilege modes, User vs Kernel space
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 2-minute clear comparison focusing on privilege rings, context transition overhead, and buffering.

### Model Answer
A **Library Call** (such as `strlen`, `std::sort`, or `memcpy`) is a standard function invocation that executes entirely within **User Space (Ring 3)**. The CPU stays in the same privilege mode, uses the application stack, and incurs trivial overhead (~few nanoseconds).

A **System Call** (such as `read`, `write`, `fork`, or `mmap`) is a programmatic request for kernel services that requires transitioning into **Kernel Space (Ring 0)**.
- It triggers a software trap instruction (`syscall` on x86-64, `svc` on ARM64).
- The CPU switches to a privileged kernel stack, saves register state, validates pointer arguments, and dispatches to the system call table.
- Because of privilege transitions, register saving, and potential cache/pipeline disruption, a system call incurs significantly higher overhead (~50–200 ns).
- Library calls often wrap system calls to reduce frequency (e.g. `fwrite` buffers user data in user space before making an expensive `write` system call).

### Follow-up Questions
1. *What is the vDSO (virtual Dynamic Shared Object)?* (A Linux kernel optimization that maps read-only kernel data like `gettimeofday` into user space, allowing syscall-free, sub-nanosecond time queries).

---

## Question 3: What happens when a program accesses an invalid memory address (e.g. dereferencing `nullptr`)?

* **Difficulty**: Medium
* **Importance**: High
* **Category**: Hardware & Memory Architecture
* **Classification**: Common industry interview theme
* **Role Relevance**: Systems, C++, Embedded, Performance Engineers
* **Prerequisites**: Virtual memory, Page tables, MMU, Signals
* **Estimated Preparation Time**: 15 minutes
* **Expected Answer Depth**: 3-minute explanation tracing the path from CPU MMU to OS signal handler.

### Model Answer
1. **CPU Memory Management Unit (MMU) Lookup**:
   - When the instruction `mov eax, [0x0]` executes, the CPU MMU attempts to translate virtual address `0x0` to a physical address using the Translation Lookaside Buffer (TLB) and multi-level page tables referenced by register `CR3`.
2. **Page Fault Exception Triggered**:
   - Address `0x0` has no valid page table entry (PTE is marked not-present or invalid).
   - The CPU halts the executing instruction and fires a hardware **Page Fault Exception (Vector 14)**, switching privilege to Ring 0.
   - The faulting address is written to CPU register `CR2`.
3. **Kernel Page Fault Handler**:
   - The kernel's `do_page_fault()` routine checks the process's list of Virtual Memory Areas (VMAs).
   - If the address falls within a valid region (e.g., stack growth or unallocated lazy page), the kernel allocates a physical frame and resumes execution.
   - For `0x0`, no valid VMA exists (null-pointer guard page). The access is illegal.
4. **Signal Delivery**:
   - The kernel constructs a `SIGSEGV` (Segmentation Violation) signal and posts it to the faulting thread's pending signal queue.
   - When transitioning back to user space, the kernel forces the thread to execute its `SIGSEGV` handler.
   - If no custom handler is registered, the default signal action terminates the process and optionally dumps core.
