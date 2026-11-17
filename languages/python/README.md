# Python: FAANG Technical Interview Preparation Curriculum

A high-yield, internal-focused Python curriculum designed for software engineers interviewing for backend, systems, and distributed infrastructure roles at **Meta, Google, Stripe, Netflix, and Amazon**.

Focuses on the Python data model, CPython internals, memory management, the GIL, asynchronous concurrency, and production patterns.

---

## 📚 Curriculum Structure

| Module | Core Concepts | High-Yield Interview Focus |
|---|---|---|
| **[01-data-model-and-dunder-methods](01-data-model-and-dunder-methods/)** | Mutability, pass-by-object-reference, `__new__` vs `__init__`, dunder catalog. | Mutable default arguments trap; hashability vs equality. |
| **[02-cpython-internals-and-gc](02-cpython-internals-and-gc/)** | PyObject header, reference counting, generational cyclic GC, `pymalloc`. | How circular references leak without GC; memory profiling. |
| **[03-gil-and-concurrency](03-gil-and-concurrency/)** | Global Interpreter Lock, CPU vs I/O bound tasks, Threading vs Multiprocessing, Python 3.13 Free-threaded (PEP 703) & JIT (PEP 744). | Why threading doesn't speed up CPU-bound code; GIL release during I/O. |
| **[04-asyncio-and-structured-concurrency](04-asyncio-and-structured-concurrency/)** | Event loop, coroutines, `async`/`await`, `asyncio.TaskGroup`, cooperative multitasking. | Event loop starvation; thread vs async memory footprint. |
| **[05-decorators-generators-and-metaprogramming](05-decorators-generators-and-metaprogramming/)** | Closures, function & class decorators, generator pipelines (`yield`), descriptors. | Writing generic decorators with `functools.wraps`; lazy evaluation. |
| **[06-high-frequency-interview-bank](06-high-frequency-interview-bank/)** | Curated FAANG Python interview questions with battle-tested answers. | Meta, Stripe, and Google Python technical interview traps. |

---

## 🛠️ Python Capstone Projects (`projects/`)

1. **`01-async-rate-limiter`**: Token-bucket algorithm with `asyncio.sleep` and sliding window.
2. **`02-lru-cache-decorator`**: Custom LRU cache implementation from scratch using `collections.OrderedDict` or Doubly Linked List + Hash Map.
3. **`03-thread-safe-connection-pool`**: Reusable database connection pool with timeouts and health checks.
4. **`04-custom-context-manager-and-profiler`**: Exception-safe resource manager and CPU execution profiler.
