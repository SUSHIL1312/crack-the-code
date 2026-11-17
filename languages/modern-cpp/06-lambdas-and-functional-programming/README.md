# Module 06: Lambdas and Functional Programming in C++

## 1. Closure Internals
A lambda is syntactic sugar for a compiler-generated anonymous `struct`:
- Value captures (`[x]`) become member variables initialized via copy.
- Reference captures (`[&x]`) become raw pointer / reference members.
- `operator()` is `const` by default. Adding `mutable` removes the `const` qualifier, allowing mutations to captured state.

---

## 2. Stateless Lambdas & Function Pointer Decay
A lambda with an empty capture list `[]` is stateless.
The C++ standard mandates that stateless lambdas implicitly convert to a raw C function pointer:
```cpp
auto add = [](int a, int b) { return a + b; };
int (*fp)(int, int) = add; // Implicit decay!
```

---

## 3. `std::function` vs Generic Templates (SBO)
- `std::function` is a type-erased wrapper that supports any callable.
  - Closures smaller than 16-32 bytes fit in its **Small Buffer Optimization (SBO)** buffer.
  - Closures larger than the buffer trigger **dynamic heap allocation** (`operator new`)!
  - Invoking `std::function` requires an indirect virtual call (cannot be inlined by the compiler).
- **In performance-critical code**: Use generic templates (`template <typename F> void invoke(F&& f)`) which inline 100% with zero heap allocation and zero branch misprediction!
