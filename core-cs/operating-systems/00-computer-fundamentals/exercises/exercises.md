# Exercises: Computer Fundamentals & Program Execution

Test your understanding of hardware modes, binary structures, memory layouts, and program startup. Attempt each exercise independently before checking the reference solutions in [solutions.md](../solutions/solutions.md).

---

## 🟢 Beginner Level

### Exercise 1.1: Section Identification
Given the following C++ declaration snippets, state which executable section (`.text`, `.rodata`, `.data`, `.bss`, Stack, or Heap) each variable or entity resides in:
1. `static int counter = 10;` (declared inside a function)
2. `const double PI = 3.1415926535;` (global)
3. `int buffer[1024];` (declared inside `main()`)
4. `static int cache[4096];` (global, uninitialized)
5. `int* p = new int(50);` (specifically, where does `p` live, and where does `*p` live?)

### Exercise 1.2: System Call Identification
Which of the following operations require a transition from User Space (Ring 3) into Kernel Space (Ring 0)?
1. Multiplying two 64-bit integers (`a * b`).
2. Calling `strlen("hello")`.
3. Reading 512 bytes from a file using `read(fd, buf, 512)`.
4. Accessing an element of `std::vector<int>` that already has capacity.
5. Invoking `std::this_thread::sleep_for(std::chrono::milliseconds(100))`.

---

## 🟡 Intermediate Level

### Exercise 2.1: Binary Footprint Investigation
Write a C++ program that defines a global uninitialized array of 10 million integers (`int large_array[10'000'000];`).
1. Compile the binary and inspect its file size on disk using `ls -lh`.
2. Modify the array to initialize the first element to non-zero: `int large_array[10'000'000] = {1};`.
3. Recompile and inspect the file size again.
4. Explain the dramatic difference in disk size in terms of `.bss` vs `.data`.

---

## 🔴 Advanced Level

### Exercise 3.1: Pre-Main Execution & Binary Hijacking
In modern C++, constructors of global objects execute before `main()`, and destructors execute after `main()`.
1. Write a program where `main()` does nothing except `return 0;`, yet the program prints:
   ```
   [Pre-main] Initializing subsystems...
   [Post-main] Tearing down subsystems...
   ```
2. Explain how the ELF `.init_array` and `.fini_array` sections enable this behavior.
3. How can an attacker abuse this mechanism in shared library injection attacks (`LD_PRELOAD`)?
