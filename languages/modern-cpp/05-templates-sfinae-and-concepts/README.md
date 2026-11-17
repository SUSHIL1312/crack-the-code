# Module 05: Templates, SFINAE & C++20 Concepts

## 1. What is SFINAE?
**SFINAE** stands for **Substitution Failure Is Not An Error**.
- When the compiler tries to deduce template arguments for a function overload, if a substitution fails in the signature, the compiler simply drops that overload from the candidate set without issuing a compilation error.
- Historically implemented using `std::enable_if_t`:
```cpp
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void process(T val); // Only participates in overload resolution if T is integer
```

---

## 2. C++20 Concepts (`concept` and `requires`)
C++20 obsoletes messy SFINAE with first-class, readable language constraints:
```cpp
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <Numeric T>
T add(T a, T b) { return a + b; }
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
