# Module 08: Modern C++ Vocabulary Types (C++17 / C++20 / C++23)

## 1. `std::string_view` and `std::span` (Zero-Copy Views)
- **`std::string_view`**: A non-owning window `(const char*, size_t)` into a string.
  - Zero heap allocation when slicing substrings!
  - *Hazard*: Lifetime! Never return a `string_view` pointing to a destroyed temporary `std::string`.
- **`std::span` (C++20)**: A non-owning view `(T*, size_t)` over contiguous elements (`vector`, raw C-array, `std::array`).

---

## 2. `std::optional` and `std::variant`
- **`std::optional<T>`**: Represents value or null state without dynamic memory allocation (`nullptr`).
- **`std::variant<Types...>`**: Type-safe discriminated union. Access via `std::visit` and `std::holds_alternative`.

---

## 3. `std::expected<T, E>` (C++23)
Deterministic error handling without exception overhead:
- Returns either a valid result `T` or an unexpected error `E`.
- Supports monadic chaining: `.and_then()`, `.transform()`, `.or_else()`.

---

## 4. Spaceship Operator `<=>` (C++20)
Defaulting `auto operator<=>(const T&) const = default;` automatically generates all 6 relational operators (`<`, `<=`, `>`, `>=`, `==`, `!=`).
