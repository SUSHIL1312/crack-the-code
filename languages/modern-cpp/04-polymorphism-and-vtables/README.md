# Module 04: Polymorphism and Vtables Under the Hood

## 1. Dynamic Dispatch Mechanics
When a class contains at least one `virtual` function:
1. The compiler inserts an implicit hidden pointer (`__vptr`) as the first member of the object (`offset 0`).
2. The compiler creates a static array of function pointers in the `.rodata` section called the **Virtual Method Table (vtable)**.
3. Every virtual function call:
   ```cpp
   base_ptr->speak();
   ```
   is transformed into a two-level pointer indirection:
   ```assembly
   movq (%rdi), %rax       # Load vptr from object (offset 0)
   call *(%rax)           # Call function pointer at vtable slot 0
   ```

---

## 2. Why Virtual Destructors are Mandatory
If a base class has non-virtual destructors:
```cpp
Base* p = new Derived();
delete p; // ❌ UNDEFINED BEHAVIOR!
```
- Because the destructor is non-virtual, the compiler resolves `p->~Base()` statically at compile time using the type of `Base*`.
- The `Derived` destructor is **never executed**!
- Any heap memory, open file handles, or network sockets held by `Derived` leak permanently!

---

## 3. Multiple Inheritance & Pointer Adjustment
In multiple inheritance (`class Derived : public BaseA, public BaseB`):
- `Derived` has **two vptrs**:
  - `vptr_A` at offset 0.
  - `vptr_B` at offset `sizeof(BaseA)`.
- When casting `Derived*` to `BaseB*`, the compiler performs pointer adjustment:
  ```cpp
  BaseB* b = d; // Adds +sizeof(BaseA) bytes to pointer address!
  ```
- This `this-pointer adjustment` is a classic FAANG interview question.
