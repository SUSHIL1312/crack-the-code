#include <iostream>
#include <memory>
#include <string>
#include <cassert>

class Animal {
public:
    std::string name;
    Animal(std::string n) : name(std::move(n)) {}
    virtual ~Animal() = default;

    virtual std::string speak() const { return "Generic Sound"; }
    virtual std::unique_ptr<Animal> clone() const = 0; // Pure virtual constructor idiom
};

class Dog : public Animal {
public:
    int fetch_count{42};
    Dog(std::string n, int fc) : Animal(std::move(n)), fetch_count(fc) {}

    std::string speak() const override { return "Woof! (" + std::to_string(fetch_count) + ")"; }

    std::unique_ptr<Animal> clone() const override {
        return std::make_unique<Dog>(*this);
    }
};

// Function taking polymorphic reference (NO SLICING)
std::string call_by_ref(const Animal& a) {
    return a.speak();
}

// Function attempting polymorphic cloning via clone idiom
std::string clone_and_speak(const Animal& a) {
    std::unique_ptr<Animal> duplicate = a.clone();
    return duplicate->speak();
}

int main() {
    std::cout << "=== Running Lab: Object Slicing & The Clone Idiom ===\n";

    Dog doggo("Rover", 100);

    // 1. Pass by reference retains full polymorphic behavior
    std::string ref_result = call_by_ref(doggo);
    assert(ref_result == "Woof! (100)");
    std::cout << "[PASS] Pass-by-reference retained polymorphic behavior: " << ref_result << "\n";

    // 2. Clone Idiom creates exact deep copy polymorphically via Base pointer
    const Animal* animal_ptr = &doggo;
    std::string clone_result = clone_and_speak(*animal_ptr);
    assert(clone_result == "Woof! (100)");
    std::cout << "[PASS] Virtual clone idiom produced accurate polymorphic duplicate: " << clone_result << "\n";

    // 3. Slicing demonstration using concrete base class
    class SlicableBase {
    public:
        int base_val{10};
        virtual int val() const { return base_val; }
        virtual ~SlicableBase() = default;
    };
    class SlicableDerived : public SlicableBase {
    public:
        int derived_val{99};
        int val() const override { return base_val + derived_val; }
    };

    SlicableDerived derived_inst;
    assert(derived_inst.val() == 109);

    // Pass by value explicitly slices object into a new SlicableBase stack frame
    auto slice_tester = [](SlicableBase sliced) {
        return sliced.val(); // vptr points to SlicableBase!
    };
    int sliced_val = slice_tester(derived_inst);
    assert(sliced_val == 10 && "Sliced object must report Base value, losing derived state");
    std::cout << "[PASS] Slicing mechanics verified: derived state sliced off, vptr bound to Base.\n";

    std::cout << "All Object Slicing tests passed successfully!\n\n";
    return 0;
}
