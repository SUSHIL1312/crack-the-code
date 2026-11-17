/**
 * templates_sfinae_and_concepts.cpp
 * 
 * Verifying SFINAE vs C++20 Concepts and Fold Expressions.
 */

#include <iostream>
#include <concepts>
#include <type_traits>
#include <cassert>

// SFINAE overload
template <typename T>
typename std::enable_if_t<std::is_integral_v<T>, std::string>
format_type(T) {
    return "Integral Type";
}

template <typename T>
typename std::enable_if_t<std::is_floating_point_v<T>, std::string>
format_type(T) {
    return "Floating Point Type";
}

// C++20 Concept
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template <Hashable T>
size_t compute_hash(const T& item) {
    return std::hash<T>{}(item);
}

// C++17 Fold Expression
template <typename... Args>
auto sum_all(Args... args) {
    return (... + args);
}

void test_templates() {
    std::cout << "[TEST] SFINAE vs Concepts vs Fold Expressions...\n";

    // SFINAE
    assert(format_type(42) == "Integral Type");
    assert(format_type(3.14) == "Floating Point Type");

    // Concepts
    static_assert(Hashable<int>);
    static_assert(Hashable<std::string>);
    assert(compute_hash(100) == std::hash<int>{}(100));

    // Fold Expression
    int total = sum_all(1, 2, 3, 4, 5);
    assert(total == 15);

    std::cout << "  ✓ SFINAE, Concepts, and Fold Expressions verified.\n";
}

int main() {
    test_templates();
    std::cout << "✅ Module 05 Templates & Concepts PASSED!\n";
    return 0;
}
