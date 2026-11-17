#!/usr/bin/env python3
"""
asyncio_structured_concurrency.py

Verifying Asyncio Event Loop, Coroutines, and TaskGroup Structured Concurrency.
"""

import asyncio
import time

async def fetch_service(name: str, delay: float) -> str:
    await asyncio.sleep(delay)
    return f"Response from {name}"

async def failing_service():
    await asyncio.sleep(0.01)
    raise ValueError("Upstream service failed")

async def test_task_group():
    print("[TEST] Asyncio TaskGroup Structured Concurrency...")

    start = time.perf_counter()
    async with asyncio.TaskGroup() as tg:
        t1 = tg.create_task(fetch_service("User Service", 0.05))
        t2 = tg.create_task(fetch_service("Inventory Service", 0.05))
        t3 = tg.create_task(fetch_service("Pricing Service", 0.05))

    duration = time.perf_counter() - start
    assert t1.result() == "Response from User Service"
    assert t2.result() == "Response from Inventory Service"
    assert t3.result() == "Response from Pricing Service"
    assert duration < 0.10, f"Tasks should run concurrently! Took {duration:.2f}s"
    print(f"  ✓ 3 services queried concurrently in {duration:.3f}s via TaskGroup.")

async def test_task_group_cancellation():
    print("[TEST] TaskGroup Automatic Child Cancellation on Failure...")

    cancelled_flag = False

    async def long_running_task():
        nonlocal cancelled_flag
        try:
            await asyncio.sleep(1.0)
        except asyncio.CancelledError:
            cancelled_flag = True
            raise

    try:
        async with asyncio.TaskGroup() as tg:
            tg.create_task(failing_service())
            tg.create_task(long_running_task())
    except* ValueError as eg:
        # Python 3.11+ ExceptionGroup catch syntax
        assert len(eg.exceptions) == 1

    assert cancelled_flag is True, "Sibling tasks must be cancelled when one fails!"
    print("  ✓ Sibling task automatically cancelled upon error. Zero resource leaks.")

async def main():
    await test_task_group()
    await test_task_group_cancellation()
    print("✅ Python Module 04 Asyncio & Structured Concurrency PASSED!")

if __name__ == "__main__":
    asyncio.run(main())
