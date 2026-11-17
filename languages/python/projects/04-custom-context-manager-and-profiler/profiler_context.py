#!/usr/bin/env python3
"""
profiler_context.py

Production-Quality Custom Context Manager & Execution Profiler:
- Strict __enter__ and __exit__ protocol implementation
- High-resolution wall-clock & CPU time profiling
- Exception handling, logging, and selective swallowing
- Automated verification test suite
"""

import time
from typing import Optional

class ExecutionProfiler:
    def __init__(self, name: str, suppress_exceptions: bool = False):
        self.name = name
        self.suppress = suppress_exceptions
        self.elapsed_wall = 0.0
        self.elapsed_cpu = 0.0
        self.exception_caught: Optional[Exception] = None
        self._start_wall = 0.0
        self._start_cpu = 0.0

    def __enter__(self):
        self._start_wall = time.perf_counter()
        self._start_cpu = time.process_time()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.elapsed_wall = time.perf_counter() - self._start_wall
        self.elapsed_cpu = time.process_time() - self._start_cpu

        if exc_val is not None:
            self.exception_caught = exc_val
            # Returning True suppresses/swallows the exception!
            # Returning False (or None) lets the exception propagate outward.
            return self.suppress
        return False

def test_profiler_context():
    print("[TEST] Custom Context Manager & Execution Profiler...")

    # 1. Normal execution profiling
    with ExecutionProfiler("Heavy Calculation") as profiler:
        total = sum(i * i for i in range(500_000))
        assert total > 0

    assert profiler.elapsed_wall > 0.0
    assert profiler.elapsed_cpu > 0.0
    assert profiler.exception_caught is None
    print(f"  ✓ Profiled 'Heavy Calculation': Wall={profiler.elapsed_wall:.4f}s, CPU={profiler.elapsed_cpu:.4f}s.")

    # 2. Exception swallowing when suppress=True
    with ExecutionProfiler("Faulty Block", suppress_exceptions=True) as profiler_err:
        raise KeyError("Missing lookup key")

    assert profiler_err.exception_caught is not None
    assert isinstance(profiler_err.exception_caught, KeyError)
    print("  ✓ Exception safely intercepted and swallowed via __exit__ return value.")

    # 3. Exception propagation when suppress=False
    propagated = False
    try:
        with ExecutionProfiler("Strict Block", suppress_exceptions=False):
            raise ValueError("Fatal computation error")
    except ValueError:
        propagated = True

    assert propagated is True, "Exception must propagate when suppress=False!"
    print("  ✓ Exception properly propagated when suppress=False.")
    print("✅ Python Capstone 04 Execution Profiler PASSED!")

if __name__ == "__main__":
    test_profiler_context()
