# 🐍 Python Object-Oriented Programming (OOP) & Runtime Internals

## Overview
In Python, **everything is an object**, including functions, modules, and classes themselves. Python’s OOP model is fundamentally dynamic:
- Objects are backed by CPython's `PyObject` C structure.
- Attributes are resolved dynamically at runtime through hash tables (`__dict__`) or descriptor arrays (`__slots__`).
- Multiple inheritance and method dispatch are resolved by the deterministic **C3 Linearization algorithm** (`Class.__mro__`).
- Class creation itself is programmable through **Metaclasses** and `__init_subclass__`.

Understanding these internals is essential for passing senior and staff engineering interviews at top tech companies.

---

## 📂 Curriculum Index

| Module | Title | Key Concepts & Mechanics |
| :--- | :--- | :--- |
| **[01](./01-python-data-model-and-classes.md)** | **Data Model & Object Creation** | `PyObject`, `__new__` vs `__init__`, instance vs class namespace, attribute lookup pipeline |
| **[02](./02-inheritance-mro-and-super.md)** | **Inheritance, MRO & `super()`** | C3 Linearization algorithm, Diamond Problem resolution, cooperative multiple inheritance |
| **[03](./03-polymorphism-and-duck-typing.md)** | **Polymorphism & Duck Typing** | Dynamic duck typing, Abstract Base Classes (`abc.ABC`), Static duck typing (`typing.Protocol`) |
| **[04](./04-memory-optimization-slots.md)** | **Memory Optimization with `__slots__`** | `__dict__` memory overhead, descriptor arrays, 60-70% RAM reduction, slotted inheritance gotchas |
| **[05](./05-metaclasses-and-class-creation.md)**| **Metaclasses & Dynamic Class Hooks** | `type` as metaclass, `__new__` vs `__init__` vs `__call__` lifecycle, API enforcement |
| **[06](./06-faang-tricky-interview-questions.md)** | **FAANG Tricky Interview Bank** | Top 10 high-frequency traps: default mutable arguments, MRO conflicts, name mangling |

---

## 🧪 Interactive Code Labs

All test labs in [`code/`](./code/) are battle-tested with Python 3.12+ and 3.13:
- `mro_and_super_demo.py`: Verifies C3 Linearization order, lateral method traversal via cooperative `super()`, and diamond problem handling.
- `new_vs_init_singleton.py`: Demonstrates instance interception with `__new__` for thread-safe singletons and subclassing immutable built-ins (`int`, `str`).
- `slots_memory_benchmark.py`: Benchmarks exact heap memory differences between standard `__dict__` and optimized `__slots__`.
- `test_oop_labs.py`: Comprehensive automated `unittest` suite covering all edge cases.

To execute the test suite:
```bash
cd languages/oops/python/code
make test
```
