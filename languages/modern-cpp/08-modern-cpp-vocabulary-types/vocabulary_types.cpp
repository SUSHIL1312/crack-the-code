/**
 * vocabulary_types.cpp
 * 
 * Verifying Modern C++ Vocabulary Types: string_view, span, optional, variant, and spaceship operator.
 */

#include <iostream>
#include <string_view>
#include <span>
#include <optional>
#include <variant>
#include <vector>
#include <compare>
#include <cassert>

struct SemVer {
    int major, minor, patch;
    auto operator<=>(const SemVer&) const = default;
};

void test_views() {
    std::cout << "[TEST] Zero-Copy Views (string_view & span)...\n";

    std::string text = "HTTP/1.1 200 OK";
    std::string_view sv = text;
    auto status_sub = sv.substr(9, 3); // "200" with zero allocations
    assert(status_sub == "200");

    std::vector<int> numbers = {10, 20, 30, 40, 50};
    std::span<int> sp(numbers);
    auto window = sp.subspan(1, 3); // {20, 30, 40}
    assert(window.size() == 3);
    assert(window[0] == 20);

    std::cout << "  ✓ string_view and span zero-allocation slicing verified.\n";
}

void test_optional_variant_spaceship() {
    std::cout << "[TEST] Optional, Variant, and Spaceship Operator...\n";

    // Optional
    std::optional<int> opt = 42;
    assert(opt.has_value() && *opt == 42);

    // Variant
    std::variant<int, std::string> var = "FAANG";
    assert(std::holds_alternative<std::string>(var));

    // Spaceship operator
    SemVer v1{1, 2, 0};
    SemVer v2{1, 2, 1};
    assert(v1 < v2);
    assert(v2 > v1);
    assert(v1 != v2);

    std::cout << "  ✓ Optional, Variant, and <=> total ordering verified.\n";
}

int main() {
    test_views();
    test_optional_variant_spaceship();
    std::cout << "✅ Module 08 Modern C++ Vocabulary Types PASSED!\n";
    return 0;
}
