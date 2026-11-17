CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -O2

BINS = vtable_inspection virtual_dtor_leak object_slicing_demo diamond_virtual_inheritance deep_vs_shallow_copy

.PHONY: all clean test

all: $(BINS)

vtable_inspection: vtable_inspection.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

virtual_dtor_leak: virtual_dtor_leak.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

object_slicing_demo: object_slicing_demo.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

diamond_virtual_inheritance: diamond_virtual_inheritance.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

deep_vs_shallow_copy: deep_vs_shallow_copy.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

test: all
	@echo "=========================================="
	@echo " Running C++ OOP Verification Test Suite  "
	@echo "=========================================="
	./vtable_inspection
	./virtual_dtor_leak
	./object_slicing_demo
	./diamond_virtual_inheritance
	./deep_vs_shallow_copy
	@echo "=========================================="
	@echo "   ALL C++ OOP LAB TESTS PASSED (100%)    "
	@echo "=========================================="

clean:
	rm -f $(BINS)
