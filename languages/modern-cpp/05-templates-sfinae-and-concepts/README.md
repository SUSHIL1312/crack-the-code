# Module 05: Templates, SFINAE & C++20 Concepts

<!-- Chapter Navigation Header -->
| ⬅️ [04: Polymorphism & Vtables](../04-polymorphism-and-vtables/README.md) | 🏠 [Modern C++ Hub](../README.md) | [06: Lambdas & Functional Programming](../06-lambdas-and-functional-programming/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. What is SFINAE? (The C++14 Approach)
**SFINAE** stands for **Substitution Failure Is Not An Error**.
- When the compiler tries to deduce template arguments for a function overload, if a substitution fails in the signature, the compiler simply drops that overload from the candidate set without issuing a compilation error.

### 🔴 The C++14 SFINAE Implementation:
```cpp
#include <type_traits>

// In C++14, you used std::enable_if_t to restrict templates:
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
void process_int_only(T val); // Overload enabled ONLY if T is an integral type

// Overload for floating-point types:
template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
void process_float_only(T val);
```

---

## 2. C++20 Concepts (`concept` and `requires`)
C++20 replaces cryptic SFINAE with first-class, readable language constraints:

### 🟢 The C++20 Concepts Implementation:
```cpp
#include <concepts>

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Clean syntax:
template <Numeric T>
T add(T a, T b) { return a + b; }

// Terse syntax (most common in modern codebases):
auto add(Numeric auto a, Numeric auto b) { return a + b; }
```
- If a caller passes `std::string`, the compiler immediately produces an intuitive diagnostic:
  `note: constraints not satisfied because 'std::is_arithmetic_v<std::string>' evaluated to false`.
- Compiles up to 10x faster than deeply nested SFINAE metaprogramming.

---

## 3. Fold Expressions (C++17)
Enables unpacking variadic template parameter packs without recursive helper functions:
```cpp
template <typename... Args>
auto sum(Args... args) {
    return (... + args); // Unary left fold: (((arg1 + arg2) + arg3) + ...)
}
```
Supported fold operators: `+`, `-`, `*`, `/`, `&&`, `||`, `,`.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| [04: Polymorphism & Vtables](../04-polymorphism-and-vtables/README.md) | [Modern C++ Hub](../README.md) | **[06: Lambdas & Functional Programming](../06-lambdas-and-functional-programming/README.md)** |
<!-- /Chapter Navigation Footer -->

