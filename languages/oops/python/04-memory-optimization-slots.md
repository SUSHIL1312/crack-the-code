# 🐍 Module 04: Memory Optimization with `__slots__`

<!-- Chapter Navigation Header -->
| ⬅️ [03: Polymorphism & Duck Typing](03-polymorphism-and-duck-typing.md) | 🏠 [Python OOP Guide](README.md) | [05: Metaclasses & Class Creation](05-metaclasses-and-class-creation.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

In large-scale data processing, distributed caches, and backend services handling millions of objects, standard Python classes consume massive amounts of RAM due to their dynamic dictionary architecture. **`__slots__`** eliminates this overhead.

---

## 1. Why Standard Python Instances Consume So Much RAM

By default, every Python instance stores its attributes in an internal dictionary: `self.__dict__`.

```text
[ Standard Python Object Instance ]
+-----------------------------------+
|  PyObject Header (16 bytes)       |  (ob_refcnt, ob_type)
+-----------------------------------+
|  Pointer to __dict__ (8 bytes)    | ---> [ PyDictObject in Heap ] (104 to 200+ bytes!)
+-----------------------------------+      - Hash table array
|  Pointer to __weakref__ (8 bytes) |      - Keys ("x", "y")
+-----------------------------------+      - Values (10, 20)
Total Memory: ~152 bytes per instance!
```

If your microservice holds 1,000,000 coordinate points in memory, standard objects consume **~150 MB of RAM**, mostly wasted on redundant dictionary keys!

---

## 2. The `__slots__` Architecture

Defining `__slots__` tells CPython: *"Do not create a `__dict__` or `__weakref__` for instances. Instead, allocate a fixed-size C array of pointers directly inside the object structure for these specific attributes."*

```python
class SlottedPoint:
    __slots__ = ('x', 'y')

    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y
```

### Memory Footprint with `__slots__`:
```text
[ Slotted Object Instance (Contiguous C Struct) ]
+-----------------------------------+
|  PyObject Header (16 bytes)       |  (ob_refcnt, ob_type)
+-----------------------------------+
|  Member 'x' pointer (8 bytes)     |  Points directly to PyFloatObject
+-----------------------------------+
|  Member 'y' pointer (8 bytes)     |  Points directly to PyFloatObject
+-----------------------------------+
Total Memory: ~48 bytes per instance (68% memory savings)!
```

### Performance & Memory Benefits:
1. **60–70% Memory Reduction**: Storing 1,000,000 slotted objects drops memory usage from ~150 MB to **~48 MB**.
2. **~20% Faster Attribute Access**: Reads and writes bypass dictionary hash probing and access the C struct offset directly via descriptors.

---

## 3. High-Yield Interview Gotchas with `__slots__`

### Trap 1: Dynamic Attributes Are Blocked
```python
p = SlottedPoint(1.0, 2.0)
p.z = 3.0 # AttributeError: 'SlottedPoint' object has no attribute 'z'
```
*Fix*: If you need slots *and* occasional dynamic attributes, include `'__dict__'` inside `__slots__`:
```python
__slots__ = ('x', 'y', '__dict__')
```

### Trap 2: Subclasses Must Also Declare `__slots__`
If a subclass inherits from a slotted class but does **not** declare its own `__slots__`, CPython will generate a `__dict__` for the subclass, completely wiping out the memory savings!

```python
class DerivedSlotted(SlottedPoint):
    pass # BUG: Instances of DerivedSlotted will have a __dict__!

class ProperDerived(SlottedPoint):
    __slots__ = ('z',) # Preserves zero __dict__ architecture!
```

### Trap 3: Multiple Inheritance Conflict
You cannot inherit from multiple base classes if more than one base class defines non-empty `__slots__`:
```python
class BaseA: __slots__ = ('a',)
class BaseB: __slots__ = ('b',)

# class Child(BaseA, BaseB): pass
# TypeError: multiple bases have instance lay-out conflict
```
*Why*: CPython cannot merge two distinct, conflicting C struct memory layouts into a single fixed-offset structure.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [03: Polymorphism & Duck Typing](03-polymorphism-and-duck-typing.md) | [Python OOP Guide](README.md) | **[05: Metaclasses & Class Creation](05-metaclasses-and-class-creation.md)** |
<!-- /Chapter Navigation Footer -->

