#!/usr/bin/env python3
"""
decorators_and_generators.py

Verifying Closures, Decorators with functools.wraps, and Generator Pipelines.
"""

import functools

def retry(times: int):
    """Decorator factory creating a retry wrapper."""
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            attempts = 0
            while attempts < times:
                try:
                    return func(*args, **kwargs)
                except Exception:
                    attempts += 1
                    if attempts == times:
                        raise
        return wrapper
    return decorator

def fibonacci_stream():
    """Infinite lazy generator stream."""
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b

def filter_evens(stream):
    """Generator pipeline filter."""
    for val in stream:
        if val % 2 == 0:
            yield val

def take(n: int, stream):
    """Take first n elements from a stream."""
    count = 0
    for val in stream:
        if count >= n:
            break
        yield val
        count += 1

def run_tests():
    print("[TEST] Decorator Mechanics & functools.wraps...")

    call_count = 0

    @retry(times=3)
    def flaky_network_call():
        nonlocal call_count
        call_count += 1
        if call_count < 3:
            raise ConnectionError("Network dropped")
        return "SUCCESS"

    # Verify docstring and name preservation via functools.wraps
    assert flaky_network_call.__name__ == "flaky_network_call"
    result = flaky_network_call()
    assert result == "SUCCESS"
    assert call_count == 3
    print("  ✓ Decorator retried successfully and preserved metadata via functools.wraps.")

    print("[TEST] Lazy Generator Pipeline Stream...")
    # Compose infinite pipeline: fibonacci -> filter evens -> take first 5
    pipeline = take(5, filter_evens(fibonacci_stream()))
    # Fibonacci: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 144
    # Even Fibonacci: 0, 2, 8, 34, 144
    evens = list(pipeline)
    assert evens == [0, 2, 8, 34, 144]
    print(f"  ✓ Generator pipeline extracted first 5 even fibonacci numbers: {evens}")

if __name__ == "__main__":
    run_tests()
    print("✅ Python Module 05 Decorators & Generators PASSED!")
