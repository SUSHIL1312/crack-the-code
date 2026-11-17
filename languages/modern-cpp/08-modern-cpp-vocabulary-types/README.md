# Module 08: Modern C++ Vocabulary Types (C++17 / C++20 / C++23)

## 1. `std::string_view` and `std::span` (Zero-Copy Views)

### 🔴 The C++14 Way:
```cpp
// Problem: Forces a heap allocation if caller passes a string literal ("hello")
void process_text(const std::string& str);

// Problem: Unsafe pointer + size pair, prone to buffer overflows
void process_buffer(const int* data, size_t size);
```

### 🟢 The Modern C++ Way (C++17/20):
```cpp
#include <string_view>
#include <span>

// std::string_view (C++17): 16-byte view (ptr + len). ZERO heap allocations!
void process_text(std::string_view str);

// std::span (C++20): Bounds-safe view over vector, C-array, or std::array:
void process_buffer(std::span<const int> buf);
```

---

## 2. `std::optional` and `std::variant`

### 🔴 The C++14 Way:
```cpp
// Option 1: Output parameter + boolean status
bool find_user(int id, User& out_user);

// Option 2: Raw pointer (risk of forgetting nullptr check, unclear ownership)
User* find_user(int id);
```

### 🟢 The Modern C++ Way (C++17):
```cpp
#include <optional>
#include <variant>

// std::optional: Explicit value-or-null semantics without dynamic memory!
std::optional<User> find_user(int id);

// std::variant: Type-safe discriminated union replacing dangerous C unions / void*
std::variant<int, std::string, double> response;
```

---

## 3. Spaceship Operator `<=>` (C++20)

### 🔴 The C++14 Way:
```cpp
struct Point {
    int x, y;
    // In C++14, you had to manually write up to 6 operators:
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator<(const Point& o) const { return x < o.x || (x == o.x && y < o.y); }
    // plus operator!=, operator<=, operator>, operator>= ...
};
```

### 🟢 The C++20 Way:
```cpp
#include <compare>

struct Point {
    int x, y;
    // In C++20, one line generates all 6 relational operators!
    auto operator<=>(const Point&) const = default;
};
```

---

## 4. `std::expected<T, E>` (C++23)
Deterministic error handling without exception overhead:
- Returns either a valid result `T` or an unexpected error `E`.
- Supports monadic chaining: `.and_then()`, `.transform()`, `.or_else()`.
- Replaces output parameters and error code enums.
