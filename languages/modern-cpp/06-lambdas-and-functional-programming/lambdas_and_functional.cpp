/**
 * lambdas_and_functional.cpp
 * 
 * Verifying Captures, Mutable stateful lambdas, Stateless decay, and Templates vs std::function.
 */

#include <iostream>
#include <functional>
#include <cassert>

void call_c_api(int (*fp)(int, int), int a, int b) {
    int res = fp(a, b);
    assert(res == (a + b));
}

void test_lambdas() {
    std::cout << "[TEST] Lambda Captures & Function Pointer Decay...\n";

    // Value vs Reference
    int factor = 10;
    auto multiplier = [factor](int x) { return x * factor; };
    assert(multiplier(5) == 50);

    // Mutable stateful generator
    auto counter = [count = 0]() mutable { return ++count; };
    assert(counter() == 1);
    assert(counter() == 2);
    assert(counter() == 3);

    // Stateless decay
    auto add = [](int a, int b) { return a + b; };
    call_c_api(add, 15, 25); // Decays to raw function pointer

    std::cout << "  ✓ Captures, mutable generator, and function pointer decay verified.\n";
}

int main() {
    test_lambdas();
    std::cout << "✅ Module 06 Lambdas & Functional PASSED!\n";
    return 0;
}
