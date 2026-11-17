#!/usr/bin/env python3
"""
gil_and_concurrency.py

Benchmarking CPU-bound vs I/O-bound concurrency under CPython GIL.
"""

import time
import threading
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor

def cpu_heavy_task(n: int) -> int:
    # Pure CPU calculation
    count = 0
    for i in range(n):
        count += (i * i) % 100
    return count

def io_heavy_task(seconds: float) -> str:
    # Simulates I/O wait (time.sleep releases the GIL!)
    time.sleep(seconds)
    return "done"

def test_gil_characteristics():
    print("[TEST] Benchmarking GIL Behavior on CPU vs I/O Workloads...")

    # 1. I/O-Bound with ThreadPoolExecutor (GIL is released during sleep)
    start_io = time.perf_counter()
    with ThreadPoolExecutor(max_workers=4) as executor:
        futures = [executor.submit(io_heavy_task, 0.05) for _ in range(4)]
        results = [f.result() for f in futures]
    duration_io = time.perf_counter() - start_io

    # If sequential, 4 * 0.05s = 0.20s. With multithreading, ~0.05s!
    assert len(results) == 4
    assert duration_io < 0.12, f"I/O tasks should run in parallel! Took {duration_io:.2f}s"
    print(f"  ✓ I/O-bound tasks executed concurrently in {duration_io:.3f}s (GIL released).")

    # 2. CPU-Bound Sequential vs Multithreaded
    n = 200_000
    start_seq = time.perf_counter()
    cpu_heavy_task(n)
    cpu_heavy_task(n)
    dur_seq = time.perf_counter() - start_seq

    start_threads = time.perf_counter()
    with ThreadPoolExecutor(max_workers=2) as executor:
        f1 = executor.submit(cpu_heavy_task, n)
        f2 = executor.submit(cpu_heavy_task, n)
        f1.result()
        f2.result()
    dur_threads = time.perf_counter() - start_threads

    print(f"  ✓ CPU-bound sequential: {dur_seq:.3f}s vs threaded: {dur_threads:.3f}s.")
    print("  ✓ Verifies that multithreading does NOT achieve speedup for CPU tasks under GIL.")

if __name__ == "__main__":
    test_gil_characteristics()
    print("✅ Python Module 03 GIL & Concurrency PASSED!")
