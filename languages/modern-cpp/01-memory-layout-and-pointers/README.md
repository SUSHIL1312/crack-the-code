# Module 01: Memory Layout, Alignment & Pointers

<!-- Chapter Navigation Header -->
| ⬅️ Start of Track | 🏠 [Modern C++ Hub](../README.md) | [02: RAII & Rule of Five](../02-raii-and-rule-of-five/README.md) ➡️ |
|:---|:---:|---:|
<!-- /Chapter Navigation Header -->

## 1. Struct Alignment and Padding
Under the CPU architecture (x86-64 / ARM64), data access is optimized when addresses are multiples of data word sizes:
- 1-byte (`char`, `bool`): 1-byte aligned.
- 2-byte (`short`, `int16_t`): 2-byte aligned (address % 2 == 0).
- 4-byte (`int`, `float`, `int32_t`): 4-byte aligned (address % 4 == 0).
- 8-byte (`double`, pointers, `int64_t`): 8-byte aligned (address % 8 == 0).

### The Padding Penalty:
```cpp
// ❌ Poor layout (24 bytes due to padding):
struct Unoptimized {
    char a;      // 1 byte + 7 bytes padding
    double b;    // 8 bytes
    char c;      // 1 byte + 7 bytes padding
}; // sizeof = 24 bytes!

// ✅ Optimized layout (16 bytes - 33% memory saved!):
struct Optimized {
    double b;    // 8 bytes
    char a;      // 1 byte
    char c;      // 1 byte + 6 bytes padding
}; // sizeof = 16 bytes!
```

---

## 2. Pointer Permutations Cheat Sheet
Interviewers test `const` pointer permutations to test syntactic rigor:

| Declaration | Meaning | Can change pointer? | Can change value? |
|---|---|---|---|
| `const int* p` (or `int const* p`) | Pointer to constant integer | **YES** (`p = &y`) | **NO** (`*p = 10` ❌) |
| `int* const p` | Constant pointer to integer | **NO** (`p = &y` ❌) | **YES** (`*p = 10`) |
| `const int* const p` | Constant pointer to constant integer | **NO** | **NO** |

*Rule to read pointers*: Read declarations **from right to left**!
- `int* const p`: `p` is a **const** pointer to an `int`.
- `const int* p`: `p` is a pointer to an `int` that is **const**.

---

<!-- Chapter Navigation Footer -->
| ⬅️ Previous Chapter | 🏠 Up to Track Hub | Next Chapter ➡️ |
|:---|:---:|---:|
| None (Track Start) | [Modern C++ Hub](../README.md) | **[02: RAII & Rule of Five](../02-raii-and-rule-of-five/README.md)** |
<!-- /Chapter Navigation Footer -->

