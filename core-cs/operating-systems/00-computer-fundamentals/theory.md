# Theory: Computer Fundamentals & Program Execution

## 1. What is an Operating System?

At its simplest, an Operating System (OS) is software that acts as an intermediary between computer hardware and user-level applications. It fulfills two primary roles:
1. **Resource Manager (Illusionist)**: Arbitrates access to finite physical resources (CPU cores, RAM, disk I/O, network interfaces) among competing processes, providing fair scheduling, protection, and virtualization.
2. **Extended Machine (Government)**: Abstracting messy, device-specific hardware commands behind clean, unified application programming interfaces (APIs / System Calls).

### Analogy
Think of the OS as the air traffic control tower and airport ground crew:
- The aircraft (user applications) request permission to land, take off, or taxi (requesting memory, CPU time, disk reads).
- The tower (OS kernel) enforces strict separation so no two planes collide on the runway (memory isolation and scheduling).

---

## 2. Hardware Architecture & The Memory Hierarchy

CPUs process instructions stored in registers at clock cycle speeds. Accessing data further from the CPU registers incurs latency penalties spanning multiple orders of magnitude:

| Storage Level | Typical Capacity | Typical Access Latency | Controlled By |
| :--- | :--- | :--- | :--- |
| **CPU Registers** | ~1–2 KB | ~0.5 ns (1 cycle) | Compiler / Assembly instructions |
| **L1 Cache (D/I)** | 32–64 KB per core | ~1–1.5 ns (4 cycles) | Hardware cache controller |
| **L2 Cache** | 512 KB–1 MB per core | ~3–5 ns (12–14 cycles) | Hardware cache controller |
| **L3 Cache (Shared)**| 16–64 MB | ~10–20 ns (40–60 cycles) | Hardware cache controller |
| **Main Memory (DRAM)**| 16–128 GB | ~60–100 ns (200 cycles) | OS (Virtual Memory) & Memory Controller |
| **NVMe SSD Storage** | 512 GB–4 TB | ~10–100 $\mu$s (100,000 ns) | OS File System & Disk Driver |
| **Mechanical HDD** | 1–16 TB | ~5–10 ms (10,000,000 ns) | OS File System & Disk Driver |

### Why This Matters in Systems Engineering
A single L3 cache miss that requires fetching from DRAM costs roughly $100\times$ more CPU cycles than an L1 cache hit. Writing cache-friendly algorithms (contiguous memory layout, sequential data access) dominates algorithmic runtime in production.

---

## 3. Kernel Space vs User Space & CPU Protection Rings

Modern x86-64 and ARM processors support hardware-enforced **privilege levels**:

- **x86-64 Protection Rings**:
  - **Ring 0 (Kernel Mode)**: Full, unrestricted access to all CPU instructions (e.g., modifying page table registers `CR3`, disabling interrupts `CLI`/`STI`, talking directly to I/O ports).
  - **Ring 1 & 2**: Historically designated for device drivers; unused in modern OSes.
  - **Ring 3 (User Mode)**: Restricted execution environment where user applications run. Direct hardware access and privileged instructions are trapped by the CPU.
- **ARM64 Exception Levels**:
  - **EL0**: User space applications.
  - **EL1**: Operating System Kernel.
  - **EL2**: Hypervisor (virtualization).
  - **EL3**: Secure Monitor (TrustZone).

If a Ring 3 application attempts to execute a privileged instruction (such as `in`/`out` or modifying `CR3`), the CPU triggers a **General Protection Fault (GPF)**, allowing the kernel to terminate the errant process with `SIGSEGV` or `SIGILL`.

---

## 4. System Calls vs Library Calls

Applications need kernel services to read files, write to network sockets, or allocate memory pages. They do this through **System Calls**.

| Dimension | Library Call (`printf`, `malloc`, `std::vector`) | System Call (`read`, `write`, `mmap`, `fork`) |
| :--- | :--- | :--- |
| **Execution Space** | Executes entirely in User Space (Ring 3). | Switches CPU into Kernel Space (Ring 0). |
| **Overhead** | Cheap (~few nanoseconds, standard function call). | Expensive (~50–200 ns; register saving, pipeline flush, page table isolation). |
| **Portability** | Standardized across C/C++ libraries (POSIX/ISO). | OS-kernel specific (Linux syscall numbers vs Windows NT syscalls). |
| **Buffering** | Often buffered (e.g., `libc` stdout buffer). | Direct, unbuffered kernel entry. |

---

## 5. Interrupts, Traps, and Exceptions

CPUs handle asynchronous and synchronous events via the **Interrupt Descriptor Table (IDT)**:

1. **Hardware Interrupt (Asynchronous)**: Generated externally by hardware devices (NIC received packet, keyboard key pressed, timer tick fired).
2. **Trap (Synchronous)**: Software-generated intentional transition to kernel mode. System calls use CPU trap instructions (`syscall` on x86-64, `svc` on ARM64).
3. **Exception / Fault (Synchronous)**: Unintentional CPU event caused by an executing instruction (division by zero, page fault on unmapped address). If recoverable (e.g., page fault for demand paging), the kernel handles it and resumes the instruction; if unrecoverable, the process receives a termination signal.
