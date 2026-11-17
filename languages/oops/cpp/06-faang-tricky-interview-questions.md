# ⚡ Module 06: Top 10 FAANG Tricky C++ OOP Interview Questions

<!-- Chapter Navigation Header -->
| ⬅️ [05: Static vs Dynamic Polymorphism](05-static-vs-dynamic-polymorphism.md) | 🏠 [C++ OOP Guide](README.md) | End of Track 🏁 |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

These 10 questions represent the most frequently asked, high-yield tricky C++ OOP interview questions at Google, Meta, Apple, Amazon, and Microsoft.

---

### Q1: What happens when a virtual function is called inside a constructor or destructor?

```cpp
class Base {
public:
    virtual void show() { std::cout << "Base::show\n"; }
    Base() { show(); }
};

class Derived : public Base {
public:
    void show() override { std::cout << "Derived::show\n"; }
    Derived() { show(); }
};

int main() {
    Derived d;
}
```

#### Answer:
* **Output**:
  ```text
  Base::show
  Derived::show
  ```
* **Why dynamic dispatch is disabled**:
  When `Base::Base()` executes, the `Derived` part of the object has **not yet been constructed**. Its member variables are uninitialized memory. Calling `Derived::show()` would access uninitialized memory and crash.
* **Under the Hood**:
  During `Base::Base()`, the compiler sets the object's `vptr` to point to `Base`'s vtable. Only after `Base::Base()` finishes and execution enters `Derived::Derived()` does the compiler update the `vptr` to point to `Derived`'s vtable.
* **The Pure Virtual Trap**:
  If `Base::show()` is pure virtual (`= 0`) and called in `Base()`, the program crashes at runtime with:
  `pure virtual method called` (`__cxa_pure_virtual`).

---

### Q2: What is the behavior of default arguments in virtual functions?

```cpp
class Base {
public:
    virtual void print(int x = 10) { std::cout << "Base: " << x << "\n"; }
};

class Derived : public Base {
public:
    void print(int x = 20) override { std::cout << "Derived: " << x << "\n"; }
};

int main() {
    Base* ptr = new Derived();
    ptr->print(); // TRICK QUESTION: What gets printed?
}
```

#### Answer:
* **Output**:
  ```text
  Derived: 10
  ```
* **Why**:
  **Default arguments are bound statically at compile time**, while **virtual functions are dispatched dynamically at runtime**!
* **Under the Hood**:
  The compiler sees `ptr` has static type `Base*`. It supplies the default argument from `Base::print` (`10`) at the call site at compile time. At runtime, dynamic dispatch calls `Derived::print`, resulting in `Derived::print(10)`!
* **Guideline**: Never redefine inherited default arguments in overriding virtual functions.

---

### Q3: Can a virtual function be `private`? Can a derived class override it?

```cpp
class Base {
private:
    virtual void secret() { std::cout << "Base secret\n"; }
public:
    void run() { secret(); }
};

class Derived : public Base {
private:
    void secret() override { std::cout << "Derived secret\n"; }
};

int main() {
    Base* b = new Derived();
    b->run();
}
```

#### Answer:
* **Yes!** This compiles and prints `"Derived secret"`.
* **Why**:
  - **Access specifiers (`private`/`public`) are checked at compile time** against the static type of the caller (`Base`).
  - **Virtual function dispatch occurs at runtime** using the dynamic `vtable`.
  - Inside `Base::run()`, `secret()` is accessible because `run()` is a member of `Base`. At runtime, the `vptr` dispatches to `Derived::secret()`.
* **Design Pattern**: This is the **Template Method Pattern** (or Non-Virtual Interface / NVI idiom). Public non-virtual interface methods call private virtual customization hooks.

---

### Q4: Why does `sizeof(Empty)` equal 1 byte, but with a virtual function it becomes 8 bytes?

```cpp
class Empty {};
class WithVirtual {
    virtual void f();
};
```

#### Answer:
* `sizeof(Empty)` = **1 byte**:
  The C++ standard requires every distinct object to have a unique memory address (`&a != &b`). If size were 0, arrays of empty objects would have identical pointers for every element.
* `sizeof(WithVirtual)` = **8 bytes** (on 64-bit systems):
  Declaring a virtual function injects the hidden `vptr` pointer at offset 0, which takes 8 bytes. The 1 dummy byte is dropped because the `vptr` already gives the object a non-zero size.

---

### Q5: What is Name Hiding in C++ Inheritance?

```cpp
class Base {
public:
    void compute(double x) { std::cout << "Base::compute(double)\n"; }
};

class Derived : public Base {
public:
    void compute(int x) { std::cout << "Derived::compute(int)\n"; }
};

int main() {
    Derived d;
    d.compute(3.14); // TRICK: Which compute is called?
}
```

#### Answer:
* **Output**: `Derived::compute(int)` (Truncates 3.14 to 3!)
* **Why**:
  In C++, member lookup stops at the derived class scope as soon as it finds a matching name (`compute`), **ignoring parameter types**. Overload resolution does NOT search base classes once a match is found in derived scope!
* **The Fix**: Bring the base class overloads into derived scope with `using`:
  ```cpp
  class Derived : public Base {
  public:
      using Base::compute; // Un-hides Base::compute!
      void compute(int x);
  };
  ```

---

### Q6: Can a constructor be virtual? If not, how do you achieve polymorphic construction?

#### Answer:
* **No.** A constructor cannot be virtual. Virtual function invocation requires a `vptr` pointing to a `vtable`, but the `vptr` is constructed and initialized *by* the constructor.
* **The Solution**: Use the **Virtual Constructor / Clone Idiom**:
  ```cpp
  class Base {
  public:
      virtual ~Base() = default;
      virtual std::unique_ptr<Base> clone() const = 0;
  };
  class Derived : public Base {
  public:
      std::unique_ptr<Base> clone() const override {
          return std::make_unique<Derived>(*this);
      }
  };
  ```

---

### Q7: What happens when an object is sliced in `std::vector`?

```cpp
class Base { public: virtual void identify() { std::cout << "Base\n"; } };
class Derived : public Base { public: void identify() override { std::cout << "Derived\n"; } };

std::vector<Base> list;
Derived d;
list.push_back(d);
list[0].identify();
```

#### Answer:
* **Output**: `"Base"`.
* **Why**:
  `std::vector<Base>` stores contiguous blocks of `sizeof(Base)` memory. When `d` is pushed, it is copied via `Base::Base(const Base&)` (copy constructor). Only the base subobject is copied; derived fields and the derived `vptr` are sliced off.
* **The Fix**: Store smart pointers: `std::vector<std::unique_ptr<Base>> list;`.

---

### Q8: Under Multiple Inheritance, can two base pointers to the same object have different memory addresses?

```cpp
class Base1 { int a; };
class Base2 { int b; };
class Derived : public Base1, public Base2 {};

Derived d;
Base1* p1 = &d;
Base2* p2 = &d;
```

#### Answer:
* **Yes!** `(void*)p1 != (void*)p2`.
* **Why**:
  `Derived` lays out `Base1` first at offset 0, followed by `Base2` at offset `sizeof(Base1)`. `p1` points to offset 0, while `p2` points to offset `+sizeof(Base1)`. The compiler automatically performs this pointer arithmetic during upcasting.

---

### Q9: When should you prefer `private` inheritance over composition?

#### Answer:
Prefer composition 95% of the time. Use `private` inheritance only when:
1. You need to override a virtual function of the base class without exposing the base interface publicly.
2. You need access to `protected` members of the base class.
3. You need **Empty Base Optimization (EBO)**: If the base class is empty (0 members), private inheritance ensures `sizeof(Derived) == sizeof(Derived_data)`, whereas composition would force an extra 1-8 bytes of padding.

---

### Q10: What is the undefined behavior risk of `dynamic_cast` on references vs pointers?

#### Answer:
* On a **pointer** (`dynamic_cast<Derived*>(base_ptr)`):
  If the dynamic cast fails, it returns `nullptr`. The caller can safely check `if (ptr != nullptr)`.
* On a **reference** (`dynamic_cast<Derived&>(base_ref)`):
  Since references cannot be null in C++, failure cannot return `nullptr`. Instead, it throws **`std::bad_cast`**. If unhandled, this terminates the application.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [05: Static vs Dynamic Polymorphism](05-static-vs-dynamic-polymorphism.md) | [C++ OOP Guide](README.md) | Track Completed 🏁 |
<!-- /Chapter Navigation Footer -->

