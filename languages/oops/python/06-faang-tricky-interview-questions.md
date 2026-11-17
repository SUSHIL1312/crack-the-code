# 🐍 Module 06: Top 10 FAANG Tricky Python OOP Interview Questions

<!-- Chapter Navigation Header -->
| ⬅️ [05: Metaclasses & Class Creation](05-metaclasses-and-class-creation.md) | 🏠 [Python OOP Guide](README.md) | End of Track 🏁 |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

These 10 questions test the most common traps and internal mechanics of Python Object-Oriented Programming asked at Google, Meta, Amazon, Apple, and Netflix.

---

### Q1: The Default Mutable Argument in `__init__` Trap

```python
class Node:
    def __init__(self, val, neighbors=[]):
        self.val = val
        self.neighbors = neighbors

n1 = Node(1)
n1.neighbors.append(2)

n2 = Node(3)
print(n2.neighbors) # TRICK QUESTION: What gets printed?
```

#### Answer:
* **Output**: `[2]`!
* **Why**:
  In Python, default arguments are evaluated **once at function definition time (when the module is loaded)**, NOT every time `__init__` is called.
  `n1` and `n2` both share the **exact same list in heap memory**.
* **The Fix**: Use `None` as the default sentinel:
  ```python
  def __init__(self, val, neighbors=None):
      self.val = val
      self.neighbors = [] if neighbors is None else neighbors
  ```

---

### Q2: What happens if `__new__` returns an instance of another class?

```python
class Alpha:
    def __init__(self):
        print("Alpha.__init__")

class Beta:
    def __new__(cls):
        print("Beta.__new__")
        return Alpha() # Returns an instance of Alpha!

    def __init__(self):
        print("Beta.__init__")

b = Beta()
```

#### Answer:
* **Output**:
  ```text
  Beta.__new__
  ```
* **Notice**: **Neither `Alpha.__init__` nor `Beta.__init__` is called!**
* **Why**:
  CPython's object allocation logic states:
  When `obj = cls(*args)` is invoked, `cls.__new__` is called. If the returned object is an instance of `cls`, CPython proceeds to call `obj.__init__(*args)`.
  Because `Alpha()` is not an instance of `Beta`, `Beta.__init__` is skipped. And because `Alpha()` was constructed inside `Beta.__new__` with empty parentheses, `Alpha.__init__` was already executed inside `__new__`, but no initializer is called by the outer `b = Beta()` call.

---

### Q3: What is the exact execution trace of `super()` in this diamond hierarchy?

```python
class A:
    def greet(self):
        print("A", end=" -> ")

class B(A):
    def greet(self):
        print("B", end=" -> ")
        super().greet()

class C(A):
    def greet(self):
        print("C", end=" -> ")
        super().greet()

class D(B, C):
    def greet(self):
        print("D", end=" -> ")
        super().greet()

D().greet()
```

#### Answer:
* **Output**: `D -> B -> C -> A -> `
* **Why**:
  `super()` does **not** call the direct parent of `B` (`A`). Instead, `super()` looks at the runtime MRO of the instance `D`:
  `D.__mro__ = (D, B, C, A, object)`
  When `B.greet` executes `super().greet()`, the next class in `D`'s MRO is **`C`**! Thus, execution moves laterally to `C.greet()` before finally reaching `A.greet()`.

---

### Q4: Class Attribute vs. Instance Attribute Shadowing

```python
class Counter:
    count = 0

c1 = Counter()
c2 = Counter()

c1.count += 1

print(c1.count) # ?
print(c2.count) # ?
print(Counter.count) # ?
```

#### Answer:
* **Output**:
  ```text
  1
  0
  0
  ```
* **Why**:
  `c1.count += 1` expands to `c1.count = c1.count + 1`.
  1. The right side (`c1.count + 1`) reads `Counter.count` ($0 + 1 = 1$).
  2. The assignment (`c1.count = 1`) creates a brand-new **instance variable** inside `c1.__dict__['count']`.
  3. `c2` and `Counter` remain untouched because they still reference the class variable.

---

### Q5: Is Python's Double Underscore (`__attr`) truly private?

```python
class BankAccount:
    def __init__(self, balance):
        self.__balance = balance # "Private"

acc = BankAccount(1000)
# print(acc.__balance) # AttributeError!
```

#### Answer:
* **No.** Python does not support true private encapsulation at the runtime memory level.
* **Under the Hood**:
  Any attribute prefixed with two leading underscores (and at most one trailing underscore) triggers **Name Mangling**.
  CPython rewrites `__balance` internally to: `_ClassName__attribute`.
* You can easily bypass it:
  ```python
  print(acc._BankAccount__balance) # 1000 (Fully accessible!)
  ```
* Name mangling exists solely to **prevent accidental naming collisions in subclasses**, not for security.

---

### Q6: Can you add new attributes to a slotted instance?

```python
class Point:
    __slots__ = ('x', 'y')
    def __init__(self, x, y):
        self.x = x
        self.y = y

p = Point(1, 2)
p.z = 3 # TRICK: Does this succeed?
```

#### Answer:
* **Raises `AttributeError: 'Point' object has no attribute 'z'`**.
* **Why**:
  Because `Point` defines `__slots__`, CPython does **not** allocate a `__dict__` for instances. Attributes can only be placed into the predefined C struct member slots.
* **How to allow dynamic attributes**:
  Include `'__dict__'` in `__slots__`: `__slots__ = ('x', 'y', '__dict__')`.

---

### Q7: `@staticmethod` vs `@classmethod` vs Instance Method under the Descriptor Protocol

#### Answer:
All three are implemented via the Python **Descriptor Protocol** (`__get__`):
1. **Instance Method**:
   `func.__get__(obj, type(obj))` returns a bound method with `self` pre-filled as the first argument.
2. **`@classmethod`**:
   `classmethod.__get__(obj, type(obj))` returns a bound method with the **class (`cls`)** pre-filled as the first argument, whether called on the class or an instance.
3. **`@staticmethod`**:
   `staticmethod.__get__(obj, type(obj))` returns the raw underlying function **unbound** (neither `self` nor `cls` is passed).

---

### Q8: What causes an MRO Conflict Error?

```python
class X: pass
class Y(X): pass

# class Z(X, Y): pass
```

#### Answer:
* **Raises `TypeError: Cannot create a consistent method resolution order (MRO)`**.
* **Why**:
  - Rule 1 of C3 Linearization: A subclass must always precede its parent in the MRO. Here, `Y` is a subclass of `X`, so `Y` must precede `X`.
  - Rule 2: The base class declaration order `(X, Y)` demands that `X` precede `Y`.
  - These two rules directly contradict each other. C3 linearization detects the cycle and refuses to construct the class.
* **The Fix**: Declare in the correct order: `class Z(Y, X): pass`.

---

### Q9: Nominal vs. Structural Subtyping: `isinstance` behavior

```python
from typing import Protocol, runtime_checkable

@runtime_checkable
class Greeter(Protocol):
    def greet(self) -> str: ...

class Person:
    def greet(self) -> str:
        return "Hi"

p = Person()
print(isinstance(p, Greeter))
```

#### Answer:
* **Output**: `True`.
* **Why**:
  Even though `Person` has **zero inheritance relationship** with `Greeter`, the `@runtime_checkable` decorator inspects the runtime class attributes. Because `Person` implements `greet()`, structural subtyping treats it as an instance of `Greeter`.

---

### Q10: What is the execution order of `__new__`, `__init__`, and `__call__` in a Metaclass?

```python
class Meta(type):
    def __call__(cls, *args, **kwargs):
        print("Meta.__call__")
        return super().__call__(*args, **kwargs)

class Service(metaclass=Meta):
    def __new__(cls, *args, **kwargs):
        print("Service.__new__")
        return super().__new__(cls)
    def __init__(self):
        print("Service.__init__")

s = Service()
```

#### Answer:
* **Output**:
  ```text
  Meta.__call__
  Service.__new__
  Service.__init__
  ```
* **Why**:
  When you write `Service()`, you are "calling" the class object `Service`. Since `Service` is an instance of `Meta`, Python invokes `Meta.__call__`. Inside `Meta.__call__`, it orchestrates `Service.__new__` followed by `Service.__init__`.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [05: Metaclasses & Class Creation](05-metaclasses-and-class-creation.md) | [Python OOP Guide](README.md) | Track Completed 🏁 |
<!-- /Chapter Navigation Footer -->

