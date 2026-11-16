# Exercises: Virtual Memory

---

## 🟢 Beginner Level

### Exercise 1.1: Address Breakdown & Page Table Sizing
Consider a system with 48-bit virtual addresses and a standard page size of 4 KB ($2^{12}$ bytes). Each Page Table Entry (PTE) is 8 bytes.

1. How many bits are dedicated to the **Page Offset**?
2. How many bits are dedicated to the **Virtual Page Number (VPN)**?
3. How many total virtual pages can a process theoretically address?
4. If this system used a **single-level (flat)** page table, how many bytes of physical RAM would be required to store the table for one single process?
5. Why does a multi-level page table reduce this memory footprint by $>99\%$ for typical processes?

---

## 🟡 Intermediate Level

### Exercise 2.1: Bitwise x86-64 Address Decomposition
Write a C++ function that takes a 64-bit virtual memory address (as `uintptr_t`) and prints its 4-level PML4 indexing components:
```cpp
void decompose_virtual_address(uintptr_t vaddr);
```
- Extract the 9-bit PML4 index (bits 39–47).
- Extract the 9-bit PDPT index (bits 30–38).
- Extract the 9-bit PD index (bits 21–29).
- Extract the 9-bit PT index (bits 12–20).
- Extract the 12-bit Page Offset (bits 0–11).
Verify your function on address `0x7FFF5FBFF840`.

---

## 🔴 Advanced Level

### Exercise 3.1: The Redis `BGSAVE` Copy-On-Write OOM Trap
Redis is an in-memory database. To create disk snapshots without blocking incoming client commands, Redis calls `fork()` to spawn a background child process (`BGSAVE`), relying on OS Copy-On-Write.
1. Suppose a Redis instance holds 40 GB of data in physical RAM on a server with 64 GB total RAM.
2. Redis invokes `fork()` to run `BGSAVE`.
3. While the child process is writing the 40 GB snapshot to disk (which takes several minutes), a surge of write operations arrives from clients, modifying keys across all parts of the dataset.
4. Explain why the Linux Out-Of-Memory (OOM) Killer terminates the Redis process even though `fork()` initially cost almost zero memory.
5. What Linux sysctl parameter (`vm.overcommit_memory`) controls this behavior?
