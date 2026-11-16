# Common Mistakes & Traps: Processes & Threads

---

## 1. Calling `exit()` Instead of `_exit()` in a Forked Child

### The Pitfall:
```cpp
pid_t pid = fork();
if (pid == 0) {
    // Child process work
    exit(0); // ❌ DANGEROUS: Flushes and closes standard I/O buffers!
}
```

### Why It's Dangerous:
`exit()` is a C standard library function that flushes standard I/O streams (`std::cout`, `stdout`) and executes functions registered with `atexit()`.
Because `fork()` copies the user-space I/O buffer to the child:
- If `stdout` had unflushed data before `fork()`, the child calling `exit()` flushes the buffer to the terminal or file.
- Later, when the parent calls `exit()`, it flushes the **exact same data a second time**, producing duplicated output!

### The Fix:
Always use `_exit()` or `_Exit()` (direct system calls) in child processes when not calling `execve()`:
```cpp
_exit(0); // ✅ Clean kernel exit without duplicate buffer flushes
```

---

## 2. The Unjoined `std::thread` Destructor Trap

### The Bug:
```cpp
void run_worker() {
    std::thread t([]() {
        std::cout << "Working...\n";
    });
    // Function ends, 't' goes out of scope and destructor runs!
}
```

### The Consequence:
In C++, if a `std::thread` is destructed while still **joinable** (i.e. neither `t.join()` nor `t.detach()` was called), its destructor immediately invokes `std::terminate()`, crashing the entire application with an uncatchable abort!

### The Fix:
- Always call `t.join()` if waiting for completion, or `t.detach()` if running as a daemon.
- In **C++20**, prefer `std::jthread`, which automatically requests cooperative cancellation (via stop tokens) and joins in its destructor.

---

## 3. Passing Local Stack References to Worker Threads

### The Bug:
```cpp
void start_thread() {
    int counter = 42;
    std::thread t([&counter]() { // ❌ Captures local variable by reference!
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << counter << "\n";
    });
    t.detach(); // start_thread returns immediately!
}
```

### The Consequence:
`start_thread()` returns and its stack frame is destroyed. When thread `t` wakes up 50 ms later and accesses `counter`, it reads garbage or writes to recycled stack memory, leading to catastrophic undefined behavior.

### The Fix:
Pass variables by value (`[counter]()`), or manage shared objects using `std::shared_ptr`.
