# Common Mistakes & Traps: Computer Fundamentals

---

## 1. Confusing Virtual Addresses with Physical RAM

### The Misconception:
"If two distinct processes both have a pointer pointing to address `0x7ffee4b2`, they are accessing the same memory and will corrupt each other's data."

### The Reality:
Every modern process executes within its own private **Virtual Address Space** provided by the OS and CPU MMU.
- Address `0x7ffee4b2` in Process A maps to physical RAM frame `0x10A4000`.
- Address `0x7ffee4b2` in Process B maps to physical RAM frame `0x8F52000`.
Processes are completely isolated from each other in hardware, unless they explicitly establish a shared memory region via `mmap(MAP_SHARED)` or `shm_open`.

---

## 2. Unbuffered I/O: Calling System Calls in a Tight Loop

### The Bad Code:
```cpp
// ANTI-PATTERN: Calling write() system call for every individual byte
for (char c : massive_string) {
    write(STDOUT_FILENO, &c, 1); // 1,000,000 mode-switches!
}
```

### The Impact:
A mode switch (Ring 3 $\to$ Ring 0 $\to$ Ring 3) takes ~50–100 ns. Making 1,000,000 individual system calls adds 100 ms of pure kernel transition overhead.

### The Correct Approach:
```cpp
// BEST PRACTICE: Buffer in user space, write in large chunks (e.g., 4KB or 8KB)
write(STDOUT_FILENO, massive_string.data(), massive_string.size()); // 1 syscall!
```
Standard libraries (`std::cout`, `printf`) automatically buffer output in user space before issuing a batch `write` syscall.

---

## 3. Assuming `const` Variables Always Live in `.rodata`

### The Misconception:
"If I declare `const int x = 42;`, it is always placed in the `.rodata` segment."

### The Reality:
Only **global** or **static** const variables (and string literals) reside in `.rodata`.
```cpp
void function() {
    const int local_val = 42; // Allocated on the STACK, NOT .rodata!
}
```
A local `const` variable is allocated in the function's stack frame. Modifying it through pointer casting is still Undefined Behavior (UB), but its memory resides on the stack.

---

## 4. Stack Overflow vs Heap Exhaustion

### The Difference:
- **Stack Overflow**: The stack has a fixed, relatively small size (typically 8MB on Linux, 512KB–1MB on macOS/Windows threads). Allocating huge arrays locally (`int buf[10'000'000]`) or deep unbounded recursion blows through the stack guard page instantly, resulting in an uncatchable `SIGSEGV`.
- **Heap Exhaustion**: The heap can grow dynamically up to the limits of virtual address space and physical RAM/swap. When `new` fails, it throws `std::bad_alloc`, which can be caught and handled gracefully in code.
