#!/usr/bin/env python3
"""
rate_limiter.py

Production-Quality Asynchronous Token Bucket Rate Limiter:
- Enforces requests-per-second rate limit with bursting
- Thread-safe / coroutine-safe with asyncio.Lock
- Automated verification test suite
"""

import asyncio
import time

class AsyncTokenBucketRateLimiter:
    def __init__(self, rate: float, capacity: float):
        self.rate = float(rate)         # Tokens added per second
        self.capacity = float(capacity) # Maximum burst bucket capacity
        self.tokens = float(capacity)
        self.last_updated = time.monotonic()
        self.lock = asyncio.Lock()

    async def acquire(self, tokens: float = 1.0):
        async with self.lock:
            while True:
                now = time.monotonic()
                elapsed = now - self.last_updated
                self.last_updated = now

                # Replenish tokens based on elapsed time
                self.tokens = min(self.capacity, self.tokens + elapsed * self.rate)

                if self.tokens >= tokens:
                    self.tokens -= tokens
                    return

                # Calculate required sleep duration to get remaining tokens
                deficit = tokens - self.tokens
                wait_time = deficit / self.rate
                await asyncio.sleep(wait_time)

async def test_rate_limiter():
    print("[TEST] Async Token Bucket Rate Limiter...")

    # Rate: 10 tokens per second, max burst: 2
    limiter = AsyncTokenBucketRateLimiter(rate=10, capacity=2)

    start = time.perf_counter()
    # Acquire 4 tokens
    for i in range(4):
        await limiter.acquire(1.0)

    duration = time.perf_counter() - start
    # First 2 tokens are immediate (burst capacity)
    # Next 2 tokens take ~0.2s (at 10 tokens/sec)
    assert 0.15 <= duration <= 0.30, f"Expected duration ~0.20s, got {duration:.3f}s"
    print(f"  ✓ 4 tokens acquired at 10 tokens/s in {duration:.3f}s.")
    print("✅ Python Capstone 01 Async Rate Limiter PASSED!")

if __name__ == "__main__":
    asyncio.run(test_rate_limiter())
