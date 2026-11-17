# Module 06: Python FAANG Technical Interview Question Bank

<!-- Chapter Navigation Header -->
| ⬅️ [05: Decorators, Generators & Metaprogramming](../05-decorators-generators-and-metaprogramming/README.md) | 🏠 [Python Hub](../README.md) | [Python Capstones](../projects/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

### Q1: What is the Global Interpreter Lock (GIL), and why can't Python threads speed up CPU-bound operations?
**Answer**:
- The GIL is a mutual exclusion lock in CPython that prevents multiple native OS threads from executing Python bytecodes concurrently.
- It exists because CPython’s memory management (`ob_refcnt`) is not thread-safe; without the GIL, concurrent operations would corrupt reference counts.
- For CPU-bound tasks (e.g. heavy math, image processing), threads spend significant time contending for the GIL and context switching, resulting in zero speedup (or even slower execution).
- To parallelize CPU-bound tasks in Python, use the `multiprocessing` module (separate processes with independent memory spaces and GILs) or C/C++/Rust extensions (like NumPy) that release the GIL (`Py_BEGIN_ALLOW_THREADS`).

---

### Q2: What is the difference between `__new__` and `__init__` in Python?
**Answer**:
- `__new__(cls, *args, **kwargs)` is the **constructor**. It is a static method responsible for creating and returning a new instance of the class in heap memory.
- `__init__(self, *args, **kwargs)` is the **initializer**. It receives the newly created instance as `self` and initializes its attributes. It returns `None`.
- `__new__` is used when subclassing immutable types (like `tuple`, `int`, `str`) or when implementing patterns like **Singleton** or **Object Pooling**.

---

### Q3: How does CPython detect and break circular references that reference counting cannot handle?
**Answer**:
- Reference counting immediately deallocates an object when `refcnt == 0`. However, if Object A refers to B and B refers to A, their reference counts remain at least 1 even after all outside references are deleted.
- CPython runs a **Generational Cyclic Garbage Collector** with 3 generations (Gen 0, Gen 1, Gen 2).
- The collector tracks only container objects (`list`, `dict`, `set`, custom classes).
- During collection, it decrements a trial reference count for all references inside tracked containers. If an isolated cluster of objects has zero external references, the collector identifies it as garbage and breaks the cycle to deallocate the objects.

---

### Q4: Why is using a mutable object as a default argument in a function dangerous?
**Answer**:
- In Python, default argument expressions are evaluated **once** at function definition time (when the module is imported/loaded), NOT each time the function is called.
- If you use a mutable object (like `def func(lst=[]):`), the exact same list instance is reused across subsequent invocations of that function. Modifications in one call persist into the next.
- The standard idiom is to use `None` as the default sentinel and initialize the mutable object inside the function body (`if lst is None: lst = []`).

---

### Q5: What is the difference between `is` and `==` in Python?
**Answer**:
- `==` checks for **equality of value** (calls `__eq__`). It checks whether the contents of two objects are equal.
- `is` checks for **identity of memory address** (`id(a) == id(b)`). It checks whether both variables point to the exact same object in heap memory.
- Always use `is` when comparing against singletons like `None` (`if x is None:`).

---

### Q6: How does `asyncio` achieve high concurrency with a single thread?
**Answer**:
- `asyncio` uses **Cooperative Multitasking** on a single-threaded Event Loop using OS I/O multiplexing (`epoll` on Linux, `kqueue` on macOS).
- When a coroutine reaches an `await` on an asynchronous I/O operation (socket read, sleep), it pauses execution and yields control back to the event loop.
- The event loop checks which sockets are ready for reading/writing and resumes coroutines that have ready data.
- Because coroutines are lightweight Python objects with minimal state (rather than 8MB OS thread stacks), a single process can sustain tens of thousands of concurrent network connections.

---

### Q7: What are Python Descriptors, and where are they used in the standard library?
**Answer**:
- A descriptor is any object that defines at least one of `__get__`, `__set__`, or `__delete__` methods.
- When an attribute of a class is a descriptor, attribute access on class instances triggers the descriptor methods instead of standard dictionary lookup.
- Descriptors power `@property`, `@classmethod`, `@staticmethod`, and ORMs (like Django and SQLAlchemy model fields).

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [05: Decorators, Generators & Metaprogramming](../05-decorators-generators-and-metaprogramming/README.md) | [Python Hub](../README.md) | **[Python Capstones](../projects/README.md)** |
<!-- /Chapter Navigation Footer -->

