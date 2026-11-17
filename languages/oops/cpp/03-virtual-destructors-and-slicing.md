# ⚡ Module 03: Virtual Destructors, Object Slicing, and the Clone Idiom

Two of the most common pitfalls tested in FAANG C++ technical interviews are **Virtual Destructor Omissions** (leading to memory leaks and undefined behavior) and **Object Slicing** (stripping polymorphic behavior when passing by value).

---

## 1. The Virtual Destructor Rule

### What Happens Without a Virtual Destructor?
Consider the following polymorphic code:

```cpp
class Base {
public:
    Base()  { std::cout << "Base constructed\n"; }
    ~Base() { std::cout << "Base destroyed\n"; } // NON-VIRTUAL!
};

class Derived : public Base {
private:
    int* buffer;
public:
    Derived()  { buffer = new int[1000]; std::cout << "Derived constructed\n"; }
    ~Derived() { delete[] buffer;        std::cout << "Derived destroyed\n"; }
};

int main() {
    Base* ptr = new Derived();
    delete ptr; // FATAL BUG: Undefined Behavior & Memory Leak!
}
```

### Execution Output:
```text
Base constructed
Derived constructed
Base destroyed
```

### Under the Hood Analysis:
1. `ptr` has static type `Base*`.
2. Because `~Base()` is **non-virtual**, the compiler generates a static, direct call to `Base::~Base()`.
3. `Derived::~Derived()` is **never called**!
4. The 1,000 integers allocated on the heap in `buffer` are **permanently leaked**.
5. According to the C++ Standard ([expr.delete]): Deleting an object of derived class through a pointer to a base class without a virtual destructor is **Undefined Behavior (UB)**.

### The Fix:
Always declare the base class destructor as `virtual`:
```cpp
class Base {
public:
    virtual ~Base() = default; // Clean polymorphic deletion!
};
```
Now, `delete ptr;` performs a dynamic lookup through the `vtable`, invoking `Derived::~Derived()` first (freeing `buffer`), followed automatically by `Base::~Base()`.

> [!IMPORTANT]
> **Herb Sutter's Guideline**: A base class destructor should be **either `public` and `virtual`**, or **`protected` and `non-virtual`** (preventing deletion via base pointer).

---

## 2. Object Slicing

Object slicing occurs when a derived class instance is assigned or passed by **value** to a base class variable or parameter.

```cpp
class Base {
public:
    int base_val{1};
    virtual void print() const { std::cout << "Base: " << base_val << "\n"; }
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    int derived_val{2};
    void print() const override {
        std::cout << "Derived: " << base_val << ", " << derived_val << "\n";
    }
};

void display_by_value(Base b) { // SLICING HAPPENS HERE!
    b.print();
}

void display_by_ref(const Base& b) { // NO SLICING
    b.print();
}
```

### Memory & Assembly Mechanics of Slicing:
When `display_by_value(d)` is called with a `Derived` object:
1. The compiler copies **only the `Base` subobject** (8 bytes of `Base` data) into the new stack frame allocated for parameter `b`.
2. The `derived_val` field is **sliced off and discarded**.
3. **The `vptr` is rewritten**: Because parameter `b` is genuinely a `Base` object on the stack, its `vptr` points to `Base`'s vtable.
4. Calling `b.print()` prints `"Base: 1"`, completely losing polymorphic behavior!

```text
[ Derived Object in Caller ]          [ Sliced Parameter on Stack ]
+-------------------------+            +-------------------------+
|  __vptr -> Derived VT   |  Copied    |  __vptr -> Base VT      | (Re-bound to Base!)
+-------------------------+ ---------> +-------------------------+
|  base_val = 1           |            |  base_val = 1           |
+-------------------------+            +-------------------------+
|  derived_val = 2        | (SLICED OFF & DISCARDED)
+-------------------------+
```

### Slicing in STL Containers:
Never create an STL container of base objects if you want polymorphism:
```cpp
std::vector<Base> vec;                   // BUG: Slices every Derived pushed into it!
std::vector<std::unique_ptr<Base>> vec;  // CORRECT: Preserves full polymorphic types!
```

---

## 3. The "Virtual Constructor" Idiom (The Clone Pattern)

### Why Can't a Constructor Be Virtual?
Interviewers frequently ask: *"Can a constructor be virtual in C++?"*
- **Answer**: **No.**
- **The Internal Reason**: A virtual call requires an object's `vptr` to index into its `vtable`. But the `vptr` is set up **by the constructor itself** during instantiation! Expecting a constructor to be virtual creates an impossible chicken-and-egg paradox.

### The Solution: The `clone()` Idiom
When you have a `Base*` and need to create an exact, polymorphic duplicate of the underlying derived instance without knowing its concrete type, use the **Virtual Clone Idiom**:

```cpp
#include <memory>

class Base {
public:
    virtual ~Base() = default;
    virtual std::unique_ptr<Base> clone() const = 0; // Pure virtual "constructor"
    virtual void speak() const = 0;
};

class Dog : public Base {
public:
    std::unique_ptr<Base> clone() const override {
        return std::make_unique<Dog>(*this); // Invokes Dog copy constructor
    }
    void speak() const override { std::cout << "Woof!\n"; }
};

class Cat : public Base {
public:
    std::unique_ptr<Base> clone() const override {
        return std::make_unique<Cat>(*this); // Invokes Cat copy constructor
    }
    void speak() const override { std::cout << "Meow!\n"; }
};

// Usage:
void duplicate_and_speak(const Base& original) {
    std::unique_ptr<Base> copy = original.clone(); // Polymorphic duplication!
    copy->speak();
}
```
