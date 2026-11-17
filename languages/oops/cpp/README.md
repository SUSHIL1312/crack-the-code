# ⚡ Modern C++ Object-Oriented Programming (OOP)

## Overview
C++ approaches Object-Oriented Programming from a systems perspective: **zero runtime overhead for unused features, deterministic resource management (RAII), and strict control over memory alignment and hardware cache efficiency**.

Unlike managed languages (Java, C#, Python) where every object has runtime metadata and methods are virtual by default, in C++:
- Methods are **non-virtual by default** (direct compile-time statically dispatched calls).
- Objects have **no runtime type info (RTTI) or vtable overhead** unless at least one virtual function is declared.
- Object destruction is **strictly deterministic**: destructors execute at well-defined points on stack unwind or explicit deletion.

> [!TIP]
> **Coming from C++14?** Read the **[C++14 to C++20 Migration & Translation Guide](../../modern-cpp/CPP14_TO_CPP20_BRIDGE.md)** for side-by-side code snippets translating C++14 idioms to Modern C++20!

---

## 📂 Curriculum Index

| Module | Title | Key Concepts & Mechanics |
| :--- | :--- | :--- |
| **[01](./01-four-pillars-in-cpp.md)** | **The 4 Pillars in C++** | Encapsulation, struct vs class ABI, access specifiers, PIMPL idiom, composition vs inheritance |
| **[02](./02-virtual-functions-and-vtables.md)** | **Virtual Functions & Vtables** | `vptr` at offset 0, `vtable` in `.rodata`, dynamic dispatch assembly, pure virtuals, `final`, `override` |
| **[03](./03-virtual-destructors-and-slicing.md)** | **Virtual Destructors & Slicing** | Why base destructors must be virtual, object slicing mechanics, virtual constructor idiom (`clone()`) |
| **[04](./04-multiple-inheritance-and-diamond.md)**| **Multiple Inheritance & Diamond**| Multiple `vptr`s, `this` pointer adjustment offset math, virtual inheritance (`virtual public Base`) |
| **[05](./05-static-vs-dynamic-polymorphism.md)** | **Static vs. Dynamic Polymorphism**| CRTP (Curiously Recurring Template Pattern), C++20 Concepts, zero-overhead compile-time contracts |
| **[06](./06-faang-tricky-interview-questions.md)** | **FAANG Tricky Interview Bank** | Top 10 high-frequency traps: virtual in constructors, name hiding, default arguments in virtuals |

---

## 🧪 Interactive Code Labs

All test labs in [`code/`](./code/) are battle-tested with modern C++20 flags (`-std=c++20 -Wall -Wextra -pedantic`):
- `vtable_inspection.cpp`: Inspects raw `vptr` addresses in memory and directly invokes virtual methods via function pointer arithmetic.
- `virtual_dtor_leak.cpp`: Reproduces memory leaks with non-virtual base destructors and proves leak elimination with `virtual ~Base()`.
- `object_slicing_demo.cpp`: Demonstrates the memory loss of pass-by-value object slicing and tests the polymorphic `clone()` idiom.
- `diamond_virtual_inheritance.cpp`: Inspects memory addresses, verifies virtual base offsets, and calculates `this` pointer adjustments.

To build and run all C++ test labs:
```bash
cd languages/oops/cpp/code
make test
```
