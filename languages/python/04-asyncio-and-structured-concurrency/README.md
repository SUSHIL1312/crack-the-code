# Module 04: Asyncio & Structured Concurrency

<!-- Chapter Navigation Header -->
| ⬅️ [03: GIL & Concurrency](../03-gil-and-concurrency/README.md) | 🏠 [Python Hub](../README.md) | [05: Decorators, Generators & Metaprogramming](../05-decorators-generators-and-metaprogramming/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. Cooperative Multitasking vs Preemptive Multithreading
- **Threads**: Managed preemptively by the OS kernel. Context switching occurs at arbitrary instructions. High stack memory footprint (~8MB per thread on Linux/macOS).
- **Asyncio**: Cooperative single-threaded event loop. Context switching only occurs at explicit `await` expressions. Ultralight memory footprint (~few KB per coroutine), allowing a single Python process to manage 50,000+ concurrent connections.

---

## 2. Structured Concurrency with `asyncio.TaskGroup` (Python 3.11+)
In older Python versions, launching background tasks with `asyncio.create_task` risked **orphaned tasks** (if one task failed, others continued running silently in the background, leaking network sockets or database connections).

`asyncio.TaskGroup` provides strict lexical lifetimes and automatic error cancellation:
```python
async with asyncio.TaskGroup() as tg:
    task1 = tg.create_task(fetch_user(1))
    task2 = tg.create_task(fetch_orders(1))
# Both tasks are guaranteed to complete or cancel before exiting context block!
```
- If `task1` raises an exception, `TaskGroup` automatically **cancels `task2`** immediately!
- Exceptions from all failed tasks are bundled into an `ExceptionGroup` (Python 3.11+).

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [03: GIL & Concurrency](../03-gil-and-concurrency/README.md) | [Python Hub](../README.md) | **[05: Decorators, Generators & Metaprogramming](../05-decorators-generators-and-metaprogramming/README.md)** |
<!-- /Chapter Navigation Footer -->

