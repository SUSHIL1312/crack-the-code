# Visual Explanation: 4-Level Page Table Translation & Copy-On-Write

## 1. The 48-bit x86-64 Paging Walk (PML4 Architecture)

```
Virtual Address (48 bits):
 47        39 38        30 29        21 20        12 11          0
┌───────────┬───────────┬───────────┬───────────┬──────────────┐
│ PML4 (9b) │  PDPT(9b) │   PD (9b) │   PT (9b) │ Offset (12b) │
└─────┬─────┴─────┬─────┴─────┬─────┴─────┬─────┴──────┬───────┘
      │           │           │           │            │
 CR3  ▼           │           │           │            │
┌───┐             │           │           │            │
│   │──►[PML4]    │           │           │            │
└───┘   │         │           │           │            │
        └──Entry──►[PDPT]     │           │            │
                   │          │           │            │
                   └──Entry───►[PD]       │            │
                              │           │            │
                              └──Entry────►[PT]        │
                                          │            │
                                          └──Entry─────┼────────┐
                                                       ▼        ▼
                                          Physical: [FrameBase | Offset]
```

---

## 2. Copy-on-Write (COW) Memory Page Splitting

### Step 1: Immediately After `fork()`
Both parent and child share the same physical frame. Both PTEs are marked **Read-Only**.

```
Parent Virtual Page 0x4000 (R/O) ──┐
                                   ├──► Physical Frame 0xA000 (Ref Count = 2)
Child Virtual Page 0x4000 (R/O) ───┘
```

---

### Step 2: Child Writes to Address `0x4000`
1. Hardware Page Fault fires on write.
2. Kernel allocates fresh Physical Frame `0xB000`.
3. Kernel copies 4096 bytes from `0xA000` $\to$ `0xB000`.
4. Child PTE updated to `0xB000` with **Read-Write** permission.

```
Parent Virtual Page 0x4000 (R/W) ─────► Physical Frame 0xA000 (Ref Count = 1)

Child Virtual Page 0x4000 (R/W) ──────► NEW Physical Frame 0xB000 (Ref Count = 1)
```
Parent and child now have private, independent physical copies.
