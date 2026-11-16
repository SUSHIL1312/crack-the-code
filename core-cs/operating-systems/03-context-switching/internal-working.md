# Internal Working: Kernel `context_switch()` and Hardware Register Swapping

## 1. The Core Kernel Dispatcher: `__schedule()`

Inside the Linux kernel, every context switch converges on `__schedule()` (in `kernel/sched/core.c`):

```c
static void __sched notrace __schedule(unsigned int sched_mode) {
    struct task_struct *prev, *next;
    struct rq *rq;

    rq = this_rq();
    prev = rq->curr;

    // 1. Pick the next runnable task from CFS/EEVDF or Real-Time queue
    next = pick_next_task(rq, prev, &rf);

    if (likely(prev != next)) {
        rq->nr_switches++;
        // 2. Perform the physical context switch
        rq = context_switch(rq, prev, next, &rf);
    }
}
```

---

## 2. Anatomy of `context_switch(rq, prev, next)`

The actual context switch performs two distinct operations:

```c
static __always_inline struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
               struct task_struct *next, struct rq_flags *rf)
{
    // Part A: Switch Virtual Memory Address Space
    if (next->mm) { // User space thread
        switch_mm_irqs_off(prev->active_mm, next->mm, next);
    }

    // Part B: Switch CPU Register State & Stack
    switch_to(prev, next, prev);

    return rq;
}
```

---

## 3. Part A: Switching Address Spaces (`switch_mm`)

When switching between two threads of the same process:
- `prev->mm == next->mm`.
- `switch_mm` takes an immediate fast path and does **nothing**.
- The page directory pointer (`CR3` register) remains unchanged.

When switching between two distinct processes:
- The CPU must load the new process's page table directory base into `CR3`:
  ```asm
  mov cr3, rax   ; Loads new PML4 base address into CR3
  ```
- **Historical Cost**: Writing to `CR3` automatically flushed the entire CPU Translation Lookaside Buffer (TLB), destroying all cached virtual-to-physical address translations.

### Modern Hardware Optimization: PCID / ASID
Modern x86-64 CPUs support **PCID (Process Context Identifiers)**, and ARM64 supports **ASID (Address Space Identifiers)**:
- Hardware associates each TLB entry with a 12-bit tag (0 to 4095) representing the process.
- When `CR3` is updated with bit 63 set (`NOFLUSH`), the CPU switches page tables **without flushing the TLB**.
- TLB entries from Process A remain valid in hardware cache so that when Process A is switched back, its translations are instantly available.

---

## 4. Part B: Switching CPU Register State (`switch_to`)

The register switch is executed in optimized assembly (`arch/x86/entry/entry_64.S`):

```asm
ENTRY(__switch_to_asm)
    /* 1. Save callee-saved registers of 'prev' onto its kernel stack */
    pushq   %rbp
    pushq   %rbx
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    /* 2. Save old kernel stack pointer into prev->thread.sp */
    movq    %rsp, TASK_threadsp(%rdi)

    /* 3. Load new kernel stack pointer from next->thread.sp into RSP */
    movq    TASK_threadsp(%rsi), %rsp

    /* 4. Restore callee-saved registers of 'next' from its kernel stack */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rbp

    /* 5. Return instruction jumps to next's instruction pointer! */
    ret
END(__switch_to_asm)
```
*The Magic of `ret`*: Because step 3 swapped the Stack Pointer (`RSP`) to point to the new task's stack, the `ret` instruction pops the return address that was previously saved on *next's* stack, seamlessly resuming execution where the new task was previously suspended!
