# Theory: Virtual Memory & Page Table Architecture

## 1. Why Virtual Memory?

Early computers used direct physical addressing: an instruction `load 0x1000` accessed physical DRAM address `0x1000`. This made multitasking hazardous—a buggy program could overwrite the OS or steal memory from neighboring programs.

Virtual Memory solves four foundational systems problems:
1. **Memory Protection & Isolation**: Processes cannot read or write to another process's memory because their virtual addresses map to completely distinct physical frames.
2. **Simplified Memory Layout**: Every 64-bit program perceives a uniform, contiguous address space (starting from `0x400000` up to `0x7fffffffffff`), regardless of where physical RAM is available or fragmented.
3. **Hardware-Enforced Permissions**: Individual pages carry hardware permission bits (Read, Write, Execute). Writing to code (`.text`) or executing data on the heap immediately triggers an MMU trap.
4. **Memory Overcommitment & Demand Paging**: An application can request 100 GB of virtual address space; physical RAM is only allocated page-by-page when the application actually writes to it.

---

## 2. Virtual to Physical Translation: Pages and Frames

Memory is divided into fixed-size chunks:
- **Virtual Page**: A chunk in virtual address space (typically **4 KB** $= 4096$ bytes).
- **Physical Frame**: A chunk in physical DRAM of identical size (4 KB).

A 64-bit virtual address is split into two components:
```
64-Bit Virtual Address:
┌───────────────────────────────────────┬───────────────────────────────┐
│ Virtual Page Number (VPN)             │ Page Offset                   │
│ [Indexes the Page Table]              │ [12 Bits = 0 to 4095 Bytes]   │
└───────────────────────────────────────┴───────────────────────────────┘
```
The CPU's Memory Management Unit (MMU) translates the **Virtual Page Number (VPN)** into a **Physical Frame Number (PFN)** using the page table, while keeping the lower 12-bit **Offset** identical:

$$\text{Physical Address} = (\text{Physical Frame Number} \ll 12) \mid \text{Offset}$$

---

## 3. Anatomy of a Page Table Entry (PTE)

Each entry in a page table is typically 64 bits (8 bytes) on x86-64:

```
63  62                                     12 11 9 8 7 6 5 4 3 2 1 0
┌──┬─────────────────────────────────────────┬───┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
│XD│ Physical Frame Base Address (40 bits)   │IGN│G│S│D│A│C│W│U│W│P│
└──┴─────────────────────────────────────────┴───┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
```

- **P (Present / Valid)**: 1 if the page resides in physical DRAM; 0 if swapped to disk or not yet allocated. If 0, accessing it causes a **Page Fault**.
- **W (Read/Write)**: 1 for read-write; 0 for read-only. Writing to a 0 page causes a protection fault.
- **U (User/Supervisor)**: 1 allows User Space (Ring 3) access; 0 restricts access to Kernel Space (Ring 0).
- **A (Accessed)**: Set by CPU hardware whenever the page is read or written (used by LRU page replacement).
- **D (Dirty)**: Set by CPU hardware whenever the page is written to (indicates the page must be written back to disk/swap if evicted).
- **XD / NX (Execute-Disable / No-Execute, Bit 63)**: 1 prevents CPU from executing instructions from this page (prevents buffer-overflow shellcode exploits on stack and heap).

---

## 4. Why Multi-Level Page Tables?

### The Math: Why a Single-Level Table is Impossible on 64-Bit
- A 48-bit virtual address space has $2^{48}$ bytes $= 256\text{ TB}$.
- With 4 KB ($2^{12}$) pages, there are $\frac{2^{48}}{2^{12}} = 2^{36}$ pages ($68.7\text{ billion}$ pages).
- If each PTE is 8 bytes, a single flat page table would require:
  $$2^{36} \times 8\text{ bytes} = 512\text{ GIGABYTES of RAM just to store the page table for ONE process!}$$

### The Solution: Multi-Level Paging (PML4)
In modern x86-64, the 48-bit address is split into 4 levels of 9 bits each ($2^9 = 512$ entries per table):
```
47        39 38        30 29        21 20        12 11          0
┌───────────┬───────────┬───────────┬───────────┬──────────────┐
│ PML4 (9b) │  PDPT(9b) │   PD (9b) │   PT (9b) │ Offset (12b) │
└───────────┴───────────┴───────────┴───────────┴──────────────┘
```
- **Huge Memory Savings**: If a region of virtual address space is unused (which is 99.9% of a 64-bit address space), the higher-level directory entries simply store `null`. **No lower-level tables are allocated!**
- A typical process only consumes a few megabytes for its active page tables.
