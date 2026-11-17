# Module 05: Decorators, Generators & Metaprogramming

## 1. Closures & Decorator Mechanics
A decorator is a callable that takes a function as an argument and returns a replacement callable.
- **`functools.wraps`**: Crucial! Copies `__name__`, `__doc__`, and `__annotations__` from original function to wrapper. Without it, introspective frameworks (FastAPI, Flask, pytest) break.
- **Closures**: Functions retaining references to free variables in the enclosing scope via `func.__closure__`.

---

## 2. Generators & Lazy Stream Pipelines
Generators (`yield`) turn functions into stateful iterators that pause execution and resume on `next()`:
- **Memory Efficiency**: Processing a 10GB log file with `[line for line in f]` consumes 10GB of RAM (OOM Crash). Using `(line for line in f)` streams line by line with $O(1)$ memory!
- **Generator Pipelines**: Chaining generators creates Unix-pipe-style streaming filters without intermediate buffer allocations.

---

## 3. Descriptors (`__get__`, `__set__`, `__delete__`)
The underlying mechanism powering properties, methods, `classmethod`, and ORMs (Django/SQLAlchemy).
If an object defines `__get__` or `__set__`, Python invokes that method whenever the attribute is accessed on an instance of the owner class.
