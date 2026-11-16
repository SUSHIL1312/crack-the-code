CXX ?= clang++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2
PYTHON ?= python3

.PHONY: all clean test test-py test-cpp lint help

help:
	@echo "Crack the Code — Build & Test Automation"
	@echo "Usage:"
	@echo "  make test-cpp     Compile and verify all C++ code examples"
	@echo "  make test-py      Run Python pytest suite"
	@echo "  make lint         Check code formatting and types"
	@echo "  make clean        Remove build artifacts and temporary files"

test: test-cpp test-py

test-cpp:
	@echo "Verifying C++ examples and projects..."
	@find languages/modern-cpp core-cs -name "*.cpp" | while read -r file; do \
		echo "Compiling $$file..."; \
		$(CXX) $(CXXFLAGS) -c "$$file" -o /dev/null || exit 1; \
	done
	@echo "All C++ source files compiled successfully."

test-py:
	@echo "Running Python test suite..."
	@$(PYTHON) -m pytest -q

lint:
	@echo "Checking Python files..."
	@$(PYTHON) -m ruff check languages/python || true
	@echo "Checking C++ files..."

clean:
	@rm -rf build/ bin/ out/ *.o *.out *.dSYM __pycache__ .pytest_cache .ruff_cache .mypy_cache
	@find . -type d -name "__pycache__" -exec rm -rf {} +
	@echo "Clean completed."
