# Module 01: Python Data Model and Dunder Methods

<!-- Chapter Navigation Header -->
| ⬅️ Start of Track | 🏠 [Python Hub](../README.md) | [02: CPython Internals & GC](../02-cpython-internals-and-gc/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. Pass-by-Object-Reference (Call by Sharing)
In Python, variables are not memory boxes; they are **names bound to objects in the heap**:
- **Immutable objects** (`int`, `float`, `str`, `tuple`, `frozenset`): Modifications produce a new object in memory.
- **Mutable objects** (`list`, `dict`, `set`, custom classes): In-place mutations modify the underlying object, visible across all references.

### 🚨 The Classic Interview Trap: Mutable Default Arguments
```python
# ❌ BUG: The default list is evaluated ONCE when the function is defined!
def append_item(item, target_list=[]):
    target_list.append(item)
    return target_list

print(append_item(1)) # [1]
print(append_item(2)) # [1, 2] -- UNINTENDED STATE RETENTION!

# ✅ FIX: Use None as default sentinel:
def append_item(item, target_list=None):
    if target_list is None:
        target_list = []
    target_list.append(item)
    return target_list
```

---

## 2. Essential Dunder Methods Catalog

| Dunder Method | Purpose | Key Details |
|---|---|---|
| `__new__(cls, ...)` | Allocates the object in memory | Static method returning a new instance. Used in singletons and immutable subclassing (`tuple`, `int`). |
| `__init__(self, ...)` | Initializes object attributes | Called after `__new__`. Returns `None`. |
| `__repr__(self)` | Unambiguous official string representation | Should look like valid Python code: `f"User(id={self.id!r})"`. For developers. |
| `__str__(self)` | Readable informal string representation | For end users. Falls back to `__repr__` if omitted. |
| `__eq__(self, other)` | Equality comparison `==` | If overridden, Python sets `__hash__ = None` unless explicitly defined! |
| `__hash__(self)` | Returns integer hash for dict/set keys | Only immutable objects whose equality never changes should be hashable. |
| `__getitem__(self, key)` | Indexing & slicing `obj[key]` | Enables square bracket indexing and automatically supports iteration! |

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| None (Track Start) | [Python Hub](../README.md) | **[02: CPython Internals & GC](../02-cpython-internals-and-gc/README.md)** |
<!-- /Chapter Navigation Footer -->

