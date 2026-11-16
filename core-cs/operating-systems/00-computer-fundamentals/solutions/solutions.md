# Solutions: Computer Fundamentals & Program Execution

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Section Identification
1. `static int counter = 10;` (inside a function): **`.data` section**. Static local variables have program lifetime and initialized non-zero values; they reside in `.data`, not on the stack.
2. `const double PI = 3.1415926535;`: **`.rodata` section**. Read-only constant data.
3. `int buffer[1024];` (inside `main()`): **Stack**. Local function variables are allocated within the caller's stack frame.
4. `static int cache[4096];` (global, uninitialized): **`.bss` section**. Uninitialized static variables are placed in `.bss` and zero-initialized by the kernel on process startup.
5. `int* p = new int(50);`:
   - `p` (the pointer variable itself) lives on the **Stack**.
   - `*p` (the integer with value 50) lives on the **Heap**.

### Exercise 1.2: System Call Identification
1. `a * b`: **No syscall** (executed directly by the CPU ALU in Ring 3).
2. `strlen("hello")`: **No syscall** (iterates over memory in Ring 3).
3. `read(fd, buf, 512)`: **Requires Syscall** (`SYS_read`). Interacts with disk/socket hardware and the kernel VFS.
4. Accessing `std::vector<int>` element: **No syscall** (direct memory indexing via CPU pointer offset).
5. `std::this_thread::sleep_for(...)`: **Requires Syscall** (`SYS_nanosleep` or `SYS_clock_nanosleep`). Suspends the thread via the kernel scheduler.

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Binary Footprint Investigation

```cpp
// Case A: Uninitialized (Lives in .bss)
int large_array[10'000'000]; // 40 MB in RAM, but consumes ~16 KB on disk!

int main() {
    large_array[0] = 5;
    return 0;
}
```

```cpp
// Case B: Explicitly initialized (Lives in .data)
int large_array[10'000'000] = {1}; // Consumes >40 MB ON DISK!

int main() {
    return 0;
}
```

### Explanation:
- In **Case A**, `large_array` is in `.bss`. The ELF binary only stores a 4-byte header entry recording: "Allocate 40,000,000 bytes at runtime". The binary size on disk remains tiny (~16 KB). When executed, the kernel maps zero-filled anonymous pages.
- In **Case B**, because the array is explicitly initialized, the compiler and linker must write all 40 million integers (the initial image) directly into the executable file on disk in the `.data` section. The binary size swells to ~40 MB.

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: Pre-Main Execution & Binary Hijacking

```cpp
#include <iostream>

struct SubsystemLifecycle {
    SubsystemLifecycle() {
        std::cout << "[Pre-main] Initializing subsystems...\n";
    }
    ~SubsystemLifecycle() {
        std::cout << "[Post-main] Tearing down subsystems...\n";
    }
};

// Global instance: constructor runs before main(), destructor runs after main() returns
static SubsystemLifecycle g_lifecycle;

int main() {
    std::cout << "[Inside main] Core workload executing.\n";
    return 0;
}
```

### Internal Working:
1. The compiler places pointers to constructors of global objects into the `.init_array` section of the ELF binary.
2. Destructors are registered with `__cxa_atexit()` or stored in `.fini_array`.
3. When `_start` executes, `__libc_start_main` iterates through function pointers in `.init_array` *before* invoking `main()`.
4. Upon return from `main()`, `exit()` iterates through the registered destructors and `.fini_array`.

### Security Implications (`LD_PRELOAD`):
An attacker can compile a shared library with a constructor (`__attribute__((constructor)) void evil() { ... }`) and force any non-setuid binary to load it via `LD_PRELOAD=/path/to/evil.so ./program`. The attacker's code will run before `main()` even begins!
