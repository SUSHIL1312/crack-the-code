# Solutions: Virtual Memory

---

## 🟢 Beginner Level Solutions

### Exercise 1.1: Address Breakdown & Sizing
1. **Offset Bits**: Page size is $4\text{ KB} = 4096 = 2^{12}$ bytes $\implies$ **12 bits** (bits 0 to 11).
2. **VPN Bits**: Total virtual address bits (48) - Offset bits (12) $\implies$ **36 bits** (bits 12 to 47).
3. **Total Virtual Pages**: $2^{36} = 68,719,476,736$ pages ($\sim 68.7$ billion pages).
4. **Single-Level Page Table Size**:
   $$\text{Table Size} = 2^{36}\text{ entries} \times 8\text{ bytes/entry} = 2^{39}\text{ bytes} = 512\text{ GIGABYTES!}$$
5. **Why Multi-Level Paging Solves This**:
   A 64-bit application rarely uses all 256 TB of address space; it typically uses a few MBs for code/data and a few MBs for stack/heap.
   In a multi-level tree (PML4), if an entire range of address space is unallocated, its higher-level directory entry is `null`. The OS never allocates the underlying page tables, reducing table RAM from 512 GB to a few kilobytes or megabytes.

---

## 🟡 Intermediate Level Solutions

### Exercise 2.1: Bitwise Address Decomposition

```cpp
#include <iostream>
#include <cstdint>
#include <iomanip>

void decompose_virtual_address(uintptr_t vaddr) {
    uintptr_t pml4_idx   = (vaddr >> 39) & 0x1FF; // 9 bits
    uintptr_t pdpt_idx   = (vaddr >> 30) & 0x1FF; // 9 bits
    uintptr_t pd_idx     = (vaddr >> 21) & 0x1FF; // 9 bits
    uintptr_t pt_idx     = (vaddr >> 12) & 0x1FF; // 9 bits
    uintptr_t offset     = vaddr & 0xFFF;         // 12 bits

    std::cout << "Decomposing Virtual Address: 0x" << std::hex << vaddr << std::dec << "\n";
    std::cout << "  PML4 Index (Bits 39-47): " << pml4_idx << "\n";
    std::cout << "  PDPT Index (Bits 30-38): " << pdpt_idx << "\n";
    std::cout << "  PD Index   (Bits 21-29): " << pd_idx   << "\n";
    std::cout << "  PT Index   (Bits 12-20): " << pt_idx   << "\n";
    std::cout << "  Page Offset(Bits 0-11):  " << offset   << " (0x" << std::hex << offset << ")\n";
}

int main() {
    decompose_virtual_address(0x7FFF5FBFF840ULL);
    return 0;
}
```

---

## 🔴 Advanced Level Solutions

### Exercise 3.1: The Redis `BGSAVE` Copy-On-Write OOM Trap
1. **Initial State**:
   - Parent Redis process holds 40 GB in RAM.
   - Server has 64 GB physical RAM.
   - `fork()` executes instantaneously using Copy-On-Write (COW). Both parent and child point to the exact same 40 GB of physical RAM.
2. **The Failure Mode**:
   - As high-volume client writes modify existing keys in the parent, the CPU triggers COW page faults.
   - For every modified 4 KB page, the kernel must allocate a **new physical frame** and duplicate the page.
   - If client writes touch even 60% of the database during the snapshot, Redis needs an additional $40\text{ GB} \times 0.60 = 24\text{ GB}$ of new physical RAM!
   - Total RAM needed $= 40\text{ GB (initial)} + 24\text{ GB (COW copies)} = 64\text{ GB}$.
   - The server completely runs out of physical memory. The Linux kernel **OOM (Out-Of-Memory) Killer** wakes up, identifies the highest memory-consuming process (Redis), and sends a fatal `SIGKILL` to terminate it!
3. **Mitigation**:
   - Set `vm.overcommit_memory = 1` in Linux sysctl (allows allocation beyond physical limits).
   - In production, rule of thumb for heavy write Redis instances: allocate at least $2\times$ memory or scale out via Redis Cluster.
