# 🐍 Module 03: Polymorphism, Duck Typing, ABCs & Protocols

<!-- Chapter Navigation Header -->
| ⬅️ [02: Inheritance, MRO & super()](02-inheritance-mro-and-super.md) | 🏠 [Python OOP Guide](README.md) | [04: Memory Optimization & __slots__](04-memory-optimization-slots.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

Python achieves polymorphism through three distinct mechanisms ranging from dynamic runtime inspection to modern static type checking: **Dynamic Duck Typing, Abstract Base Classes (ABCs), and Structural Protocols**.

---

## 1. Dynamic Duck Typing: Polymorphism Without Inheritance

In Python, functions do not specify expected types; they specify **expected behaviors**:

> *"If it walks like a duck and quacks like a duck, it is a duck."*

```python
class PostgresStorage:
    def save(self, data: dict) -> None:
        print(f"Persisting to PostgreSQL: {data}")

class S3Storage:
    def save(self, data: dict) -> None:
        print(f"Uploading to AWS S3: {data}")

class MockStorage:
    def save(self, data: dict) -> None:
        print(f"In-Memory Test: {data}")

# Polymorphic client: Has NO dependency on any base class!
def record_event(storage, event: dict):
    storage.save(event) # Dispatches purely based on the presence of .save()
```

---

## 2. Abstract Base Classes: Nominal Subtyping (`abc.ABC`)

When building enterprise frameworks or plug-in systems, you want to fail fast if a developer forgets to implement a required method. **`abc.ABC`** prevents instantiation of incomplete subclasses:

```python
from abc import ABC, abstractmethod

class BaseCache(ABC):
    @abstractmethod
    def get(self, key: str) -> bytes | None:
        """Retrieve key or return None."""
        pass

    @abstractmethod
    def set(self, key: str, value: bytes, ttl: int = 300) -> None:
        """Store key with TTL."""
        pass

class RedisCache(BaseCache):
    def get(self, key: str) -> bytes | None:
        return b"cached_val"
    # BUG: Forgot to implement 'set'!

# Attempting to instantiate RedisCache raises an immediate TypeError:
# TypeError: Can't instantiate abstract class RedisCache with abstract method set
```

### Virtual Subclasses via `register()`:
You can declare an existing class as an implementation of an ABC without changing its inheritance hierarchy:
```python
from collections.abc import Sequence

class CustomList:
    def __len__(self): return 0
    def __getitem__(self, index): raise IndexError

# Register without inheriting:
Sequence.register(CustomList)
print(isinstance(CustomList(), Sequence)) # True!
```

---

## 3. Structural Subtyping: `typing.Protocol` (PEP 544)

In modern typed Python (Python 3.8+), **Protocols** provide compile-time (and runtime) duck typing. Unlike ABCs, a class does **not** need to inherit from the Protocol:

```python
from typing import Protocol, runtime_checkable

@runtime_checkable
class Renderable(Protocol):
    def render(self) -> str:
        ...

# Concrete class: ZERO inheritance!
class MarkdownDocument:
    def render(self) -> str:
        return "# Title\nContent"

class HTMLDocument:
    def render(self) -> str:
        return "<h1>Title</h1><p>Content</p>"

def publish(doc: Renderable):
    print(doc.render())

# Mypy and IDEs verify this at compile time:
publish(MarkdownDocument()) # Valid!
publish(HTMLDocument())     # Valid!

# Runtime check works because of @runtime_checkable:
print(isinstance(MarkdownDocument(), Renderable)) # True!
```

---

## 4. Architectural Comparison: ABC vs Protocol

| Dimension | Abstract Base Class (`abc.ABC`) | Structural Protocol (`typing.Protocol`) |
| :--- | :--- | :--- |
| **Typing Philosophy** | **Nominal** (Must explicitly inherit from or register with the ABC) | **Structural** (Duck typing: matching signature is sufficient) |
| **Third-Party Classes** | Requires explicit `.register()` call | Automatic if methods match |
| **Code Sharing** | Supports concrete helper method implementation | Pure interface contract (no shared implementation) |
| **Enforcement** | Fails at runtime during `__init__` | Verified ahead-of-time by static type checkers (mypy/pyright) |

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [02: Inheritance, MRO & super()](02-inheritance-mro-and-super.md) | [Python OOP Guide](README.md) | **[04: Memory Optimization & __slots__](04-memory-optimization-slots.md)** |
<!-- /Chapter Navigation Footer -->

