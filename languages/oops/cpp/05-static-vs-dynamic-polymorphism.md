# ⚡ Module 05: Static vs. Dynamic Polymorphism (CRTP & C++20 Concepts)

<!-- Chapter Navigation Header -->
| ⬅️ [04: Multiple Inheritance & Diamond](04-multiple-inheritance-and-diamond.md) | 🏠 [C++ OOP Guide](README.md) | [06: FAANG Tricky Questions](06-faang-tricky-interview-questions.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

In high-performance C++ systems (high-frequency trading, game engines, network packet parsers), the runtime cost of virtual function indirection (~1-2 ns plus potential cache misses and branch mispredictions) is unacceptable. Modern C++ provides **Static Polymorphism** to achieve zero-overhead polymorphic dispatch.

---

## 1. The Curiously Recurring Template Pattern (CRTP)

CRTP achieves compile-time polymorphism by passing the derived class as a template parameter to the base class:

```cpp
#include <iostream>

template <typename Derived>
class BasePacketParser {
public:
    void parse_header() {
        // Statically cast 'this' to Derived type at compile-time!
        static_cast<Derived*>(this)->parse_header_impl();
    }
};

class EthernetParser : public BasePacketParser<EthernetParser> {
public:
    void parse_header_impl() {
        std::cout << "Parsing Ethernet 802.3 Header [Zero Overhead]\n";
    }
};

class IpParser : public BasePacketParser<IpParser> {
public:
    void parse_header_impl() {
        std::cout << "Parsing IPv4/IPv6 Header [Zero Overhead]\n";
    }
};

// Generic dispatch function:
template <typename T>
void process_packet(BasePacketParser<T>& parser) {
    parser.parse_header(); // Directly inlined by compiler! No vtable!
}
```

### Why CRTP Beats Virtual Functions in Low Latency:
1. **Zero Memory Overhead**: `sizeof(EthernetParser)` has no hidden `vptr` (0 extra bytes).
2. **Direct Branching**: The call is a direct `call` instruction (or completely inlined), eliminating indirect branch mispredictions in the CPU pipeline.
3. **Compiler Inlining**: The optimizer can see the full function body and inline loop vectorizations across the call site.

---

## 2. Static Interface Constraints: C++14 SFINAE vs. C++20 Concepts

How do you enforce that a template type implements specific methods (like `print()`) at compile time?

### 🔴 The C++14 Way (SFINAE & Expression Detection):
In C++14, enforcing that `T` has a `.print()` method required cryptic `std::enable_if_t` and `decltype` boilerplate:

```cpp
#include <iostream>
#include <type_traits>

// C++14: SFINAE helper checking if T has .print()
template <typename T>
class has_print {
    template <typename U>
    static auto test(int) -> decltype(std::declval<U>().print(), std::true_type());
    template <typename>
    static auto test(...) -> std::false_type;
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// Constrained function using enable_if_t:
template <typename T, typename = std::enable_if_t<has_print<T>::value>>
void render_item_cpp14(const T& item) {
    item.print();
}
```

### 🟢 The C++20 Way (Concepts & `requires`):
C++20 replaces all the SFINAE boilerplate with clear, declarative **Concepts**:

```cpp
#include <concepts>
#include <iostream>
#include <string>

// Define the compile-time interface contract in 4 readable lines:
template <typename T>
concept Printable = requires(T a) {
    { a.to_string() } -> std::same_as<std::string>;
    { a.print() } -> std::same_as<void>;
};

// Concrete class 1
class Order {
public:
    std::string to_string() const { return "Order #4819"; }
    void print() const { std::cout << to_string() << "\n"; }
};

// Concrete class 2
class User {
public:
    std::string to_string() const { return "User: Alice"; }
    void print() const { std::cout << to_string() << "\n"; }
};

// Constrained generic function:
void render_item(const Printable auto& item) {
    item.print(); // Fully verified at compile time!
}

int main() {
    Order o;
    User u;
    render_item(o); // Compiles & inlines
    render_item(u); // Compiles & inlines

    // int x = 42;
    // render_item(x); // COMPILE ERROR: constraints not satisfied!
}
```

---

## 3. Dynamic vs Static Architectural Decision Matrix

| Requirement | Use Dynamic Polymorphism (`virtual`) | Use Static Polymorphism (Concepts / CRTP) |
| :--- | :--- | :--- |
| **Heterogeneous Collections** | ✅ Yes (`std::vector<std::unique_ptr<Base>>`) | ❌ No (all elements must be identical type at compile time) |
| **Plugin Architecture** | ✅ Yes (load `.so` / `.dll` libraries at runtime) | ❌ No (all types must be known at compile time) |
| **Microsecond / Nanosecond Critical** | ❌ No (virtual call indirection overhead) | ✅ Yes (fully inlined, zero-cost abstraction) |
| **Binary Size** | ✅ Compact (single copy of compiled code) | ⚠️ Potential code bloat if instantiated for many distinct types |

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [04: Multiple Inheritance & Diamond](04-multiple-inheritance-and-diamond.md) | [C++ OOP Guide](README.md) | **[06: FAANG Tricky Questions](06-faang-tricky-interview-questions.md)** |
<!-- /Chapter Navigation Footer -->

