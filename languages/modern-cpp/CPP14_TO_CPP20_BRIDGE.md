# 🌉 The C++14 to C++20 Bridge: Migration & Translation Guide

If you are already comfortable with **C++14**, you already understand 90% of what matters in technical interviews: **memory layout, pointers, stack vs. heap, RAII, move semantics, and vtables are identical across C++14 and C++20**.

C++17 and C++20 do not invalidate your C++14 foundation; they **eliminate boilerplate, enhance type safety, and provide zero-overhead abstractions**.

This guide provides a direct side-by-side translation: **"How you wrote it in C++14" $\longrightarrow$ "How to write it in Modern C++ (C++17/C++20)"**.

---

## 📑 Quick Translation Cheat Sheet

| Intent | The C++14 Way | The Modern C++20 Way | Why It Matters in Interviews |
| :--- | :--- | :--- | :--- |
| **Pass string without copying** | `const std::string&` | `std::string_view` (C++17) | Avoids heap allocation when passing string literals `""` or substrings. |
| **Pass contiguous array/buffer** | `const T* data, size_t len` | `std::span<const T>` (C++20) | Replaces dangerous C-style pointer+length pairs with bounds-safe view. |
| **Compare structs (`<`, `==`, etc.)** | Write 6 manual `operator`s | `auto operator<=>(const T&) = default;` (C++20) | Eliminates 30 lines of boilerplate comparison code. |
| **Constrain template type** | SFINAE (`std::enable_if_t`) | `concept` and `requires` (C++20) | Human-readable compile-time error messages instead of 50-line template vomit. |
| **Return value or null** | Return pointer (`nullptr`) or out-param | `std::optional<T>` (C++17) | Expresses optionality in the type system without dynamic heap allocation. |
| **Return value or error** | Output parameter + status code | `std::expected<T, E>` (C++23) | Monadic error handling without exceptions. |
| **Type-safe union** | `union` + manual tag tracking | `std::variant<Types...>` (C++17) | Memory-safe discriminated union with `std::visit`. |
| **Unpack tuple / pair** | `std::tie(a, b) = pair;` | `auto [a, b] = pair;` (C++17) | Structured bindings: clean unpacking directly at declaration. |
| **Compile-time condition** | Template specialization / tag dispatch | `if constexpr (cond)` (C++17) | Branches discarded at compile time without SFINAE. |
| **Thread execution** | `std::thread` (crashes if not joined) | `std::jthread` (C++20) | Auto-joins on destruction (RAII) and supports cooperative cancellation. |
| **String formatting** | `snprintf` or `std::ostringstream` | `std::format` (C++20) | Python-style format strings (`"{}:{}"`) with type safety and speed. |
| **Unpack variadic args** | Recursive template functions | Fold expressions `(... + args)` (C++17) | Single-line variadic pack reduction. |

---

## 1. String Arguments: `const std::string&` vs `std::string_view`

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <string>

// Problem: If caller passes a literal ("admin"), C++ creates a temporary
// std::string on the heap, passes the reference, then destroys it!
bool is_admin(const std::string& username) {
    return username == "admin";
}

int main() {
    is_admin("admin"); // Unnecessary heap allocation and deallocation!
    
    std::string full = "user:alice";
    // Substring also forces a new string heap allocation:
    is_admin(full.substr(5)); // Another heap allocation!
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <string_view>

// std::string_view is non-owning: 16 bytes on the stack (const char* ptr + size_t len)
// ZERO heap allocations regardless of whether you pass literal, std::string, or substring!
bool is_admin(std::string_view username) {
    return username == "admin";
}

int main() {
    is_admin("admin"); // ZERO allocations! Points directly to .rodata string literal.
    
    std::string full = "user:alice";
    // Substring view is a simple pointer + length offset:
    is_admin(std::string_view(full).substr(5)); // ZERO allocations!
}
```
* **Under the Hood**: `std::string_view` is just `struct { const char* data; size_t size; }`. It lives entirely in CPU registers or on the stack.

---

## 2. Buffer / Array Views: C-Pointers vs `std::span`

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <vector>

// C-style pointer + length pair: prone to buffer overflows and mismatch bugs
void process_buffer(const int* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << "\n";
}

int main() {
    int arr[] = {1, 2, 3, 4};
    process_buffer(arr, 4);

    std::vector<int> vec = {5, 6, 7};
    process_buffer(vec.data(), vec.size()); // Awkward call
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <vector>
#include <span>

// std::span seamlessly accepts C-arrays, std::array, and std::vector with zero copy!
void process_buffer(std::span<const int> buf) {
    for (int val : buf) { // Range-for works directly!
        std::cout << val << " ";
    }
    std::cout << "\n";
}

int main() {
    int arr[] = {1, 2, 3, 4};
    process_buffer(arr); // Automatically deduces size!

    std::vector<int> vec = {5, 6, 7};
    process_buffer(vec); // Works directly without .data() or .size()!
}
```

---

## 3. Struct Comparisons: 6 Operators vs The Spaceship Operator (`<=>`)

### 🔴 The C++14 Way:
```cpp
#include <string>

struct User {
    int id;
    std::string name;

    // In C++14, you had to manually write up to 6 relational operators:
    bool operator==(const User& o) const { return id == o.id && name == o.name; }
    bool operator!=(const User& o) const { return !(*this == o); }
    bool operator<(const User& o) const {
        if (id != o.id) return id < o.id;
        return name < o.name;
    }
    bool operator<=(const User& o) const { return !(o < *this); }
    bool operator>(const User& o) const  { return o < *this; }
    bool operator>=(const User& o) const { return !(*this < o); }
};
```

### 🟢 The C++20 Way:
```cpp
#include <string>
#include <compare>

struct User {
    int id;
    std::string name;

    // In C++20, defaulting operator<=> automatically generates ==, !=, <, <=, >, and >= !
    auto operator<=>(const User&) const = default;
};
```
* **Under the Hood**: The compiler generates member-by-member lexicographical comparison at compile time.

---

## 4. Template Constraints: SFINAE (`std::enable_if_t`) vs Concepts

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <type_traits>

// SFINAE: Enable function only if T is an integral type
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
T multiply_by_two(T val) {
    return val * 2;
}

int main() {
    multiply_by_two(10); // Compiles
    // multiply_by_two("hello"); // Produces 30+ lines of incomprehensible template error!
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <concepts>

// Clean, human-readable type constraint
template <std::integral T>
T multiply_by_two(T val) {
    return val * 2;
}

// Or using terse C++20 syntax:
auto multiply_by_two_terse(std::integral auto val) {
    return val * 2;
}

int main() {
    multiply_by_two(10); // Compiles
    // multiply_by_two("hello"); // Clear error: 'constraints not satisfied: std::integral<const char*>'
}
```

---

## 5. Returning Optional Values: Sentinel / Out-Param vs `std::optional`

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <unordered_map>
#include <string>

// C++14 Option 1: Return bool, set out-parameter
bool find_email(int user_id, std::string& out_email) {
    if (user_id == 1) {
        out_email = "alice@example.com";
        return true;
    }
    return false;
}

// C++14 Option 2: Return raw pointer (requires caller to check for nullptr)
const std::string* find_email_ptr(int user_id) {
    static const std::string email = "alice@example.com";
    if (user_id == 1) return &email;
    return nullptr;
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <optional>
#include <string>

// std::optional represents a value or nothing. No pointers, no dynamic memory!
std::optional<std::string> find_email(int user_id) {
    if (user_id == 1) {
        return "alice@example.com";
    }
    return std::nullopt; // Explicit empty state
}

int main() {
    auto email = find_email(1);
    if (email) {
        std::cout << "Found: " << *email << "\n";
    }
    // Or with fallback default:
    std::cout << email.value_or("no-reply@example.com") << "\n";
}
```

---

## 6. Unpacking Pairs & Tuples: `std::tie` vs Structured Bindings

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <tuple>
#include <string>

std::pair<int, std::string> get_user() {
    return {101, "Alice"};
}

int main() {
    // C++14: Pre-declare variables and use std::tie
    int id;
    std::string name;
    std::tie(id, name) = get_user();

    std::cout << id << ": " << name << "\n";
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <string>

std::pair<int, std::string> get_user() {
    return {101, "Alice"};
}

int main() {
    // C++17/20: Structured binding declares and unpacks in one clean statement!
    auto [id, name] = get_user();
    std::cout << id << ": " << name << "\n";

    // Works with map iteration too:
    std::unordered_map<std::string, int> scores = {{"Alice", 95}};
    for (const auto& [player, score] : scores) {
        std::cout << player << " scored " << score << "\n";
    }
}
```

---

## 7. Multithreading Safety: `std::thread` vs `std::jthread`

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <thread>

void worker() {
    std::cout << "Working...\n";
}

void risky_function() {
    std::thread t(worker);

    // DANGER: If an exception is thrown here before t.join(),
    // std::terminate() is called and the ENTIRE PROCESS CRASHES!
    // In C++14, you had to write a custom RAII ThreadGuard class.
    t.join();
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <thread>

void worker() {
    std::cout << "Working...\n";
}

void safe_function() {
    // std::jthread automatically requests stop and calls .join() in its destructor!
    std::jthread t(worker);
    // If an exception occurs, t.~jthread() executes and safely joins before stack unwinds!
}
```

---

## 8. Compile-Time Branching: Tag Dispatch vs `if constexpr`

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <type_traits>

// In C++14, you had to use Tag Dispatching with helper overloads:
template <typename T>
void print_val_impl(T val, std::true_type /* is_pointer */) {
    std::cout << "Pointer value: " << *val << "\n";
}

template <typename T>
void print_val_impl(T val, std::false_type /* is_pointer */) {
    std::cout << "Direct value: " << val << "\n";
}

template <typename T>
void print_val(T val) {
    print_val_impl(val, std::is_pointer<T>{});
}
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <type_traits>

template <typename T>
void print_val(T val) {
    // if constexpr evaluates condition at COMPILE TIME!
    // The unselected branch is completely discarded and NOT compiled for type T!
    if constexpr (std::is_pointer_v<T>) {
        std::cout << "Pointer value: " << *val << "\n";
    } else {
        std::cout << "Direct value: " << val << "\n";
    }
}
```

---

## 9. String Formatting: `snprintf` / `ostringstream` vs `std::format`

### 🔴 The C++14 Way:
```cpp
#include <iostream>
#include <sstream>
#include <cstdio>

// Method 1: Clunky iostreams
std::ostringstream oss;
oss << "Host: " << "localhost" << ":" << 8080;
std::string s1 = oss.str();

// Method 2: Unsafe C-style buffer
char buf[128];
std::snprintf(buf, sizeof(buf), "Host: %s:%d", "localhost", 8080);
```

### 🟢 The C++20 Way:
```cpp
#include <iostream>
#include <format>

// Type-safe, fast, and uses clean Python-style format strings:
std::string s = std::format("Host: {}:{}", "localhost", 8080);
```

---

## 🎯 Interview Takeaway: How to Answer When Asked About C++ Standard

If your interviewer asks: *"What version of C++ are you most comfortable with?"*

> **Model Response**:  
> *"I have deep production experience with C++14, so all core systems principles—RAII, move semantics, pointer mechanics, thread safety, and vtable memory layouts—are second nature to me. In newer standards, I frequently leverage C++17/20 idioms like `std::string_view` and `std::span` to eliminate heap copies, `std::optional` to avoid sentinel values, and `auto operator<=>` or `concepts` to replace heavy boilerplate."*

This response shows confidence, solid fundamentals, and modern awareness without overselling!
