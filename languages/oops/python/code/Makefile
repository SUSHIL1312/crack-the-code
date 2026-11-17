PYTHON = python3

.PHONY: all test clean

all: test

test:
	@echo "=========================================="
	@echo " Running Python OOP Verification Suite    "
	@echo "=========================================="
	$(PYTHON) mro_and_super_demo.py
	$(PYTHON) new_vs_init_singleton.py
	$(PYTHON) slots_memory_benchmark.py
	$(PYTHON) -m unittest test_oop_labs.py -v
	@echo "=========================================="
	@echo "   ALL PYTHON OOP LAB TESTS PASSED (100%) "
	@echo "=========================================="

clean:
	rm -rf __pycache__ *.pyc
