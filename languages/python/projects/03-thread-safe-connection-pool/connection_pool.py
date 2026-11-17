#!/usr/bin/env python3
"""
connection_pool.py

Production-Quality Thread-Safe Connection Pool:
- Thread-safe acquisition and release via condition variable
- Context manager support for automatic check-in
- Timeout handling and multi-threaded stress test
"""

import threading
import time

class MockConnection:
    def __init__(self, conn_id: int):
        self.conn_id = conn_id
        self.in_use = False

    def query(self, sql: str) -> str:
        return f"Result for '{sql}' on conn #{self.conn_id}"

class ConnectionPool:
    def __init__(self, pool_size: int):
        self.pool_size = pool_size
        self._available = [MockConnection(i) for i in range(pool_size)]
        self._lock = threading.Lock()
        self._cv = threading.Condition(self._lock)

    def acquire(self, timeout: float = None) -> MockConnection:
        with self._cv:
            end_time = time.monotonic() + timeout if timeout else None
            while not self._available:
                if timeout:
                    remaining = end_time - time.monotonic()
                    if remaining <= 0:
                        raise TimeoutError("Connection pool acquisition timed out")
                    self._cv.wait(remaining)
                else:
                    self._cv.wait()

            conn = self._available.pop()
            conn.in_use = True
            return conn

    def release(self, conn: MockConnection):
        with self._cv:
            conn.in_use = False
            self._available.append(conn)
            self._cv.notify()

    def get_connection(self, timeout: float = None):
        """Context manager for automatic check-in."""
        return _ConnectionContext(self, timeout)

class _ConnectionContext:
    def __init__(self, pool: ConnectionPool, timeout: float):
        self.pool = pool
        self.timeout = timeout
        self.conn = None

    def __enter__(self):
        self.conn = self.pool.acquire(self.timeout)
        return self.conn

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.conn:
            self.pool.release(self.conn)

def test_connection_pool():
    print("[TEST] Thread-Safe Connection Pool Stress Test...")

    pool = ConnectionPool(pool_size=3)
    completed_queries = 0
    lock = threading.Lock()

    def worker(worker_id: int):
        nonlocal completed_queries
        for i in range(10):
            with pool.get_connection(timeout=2.0) as conn:
                res = conn.query(f"SELECT * FROM users WHERE id={worker_id}")
                assert "Result for" in res
                time.sleep(0.005) # Hold connection briefly
                with lock:
                    completed_queries += 1

    threads = [threading.Thread(target=worker, args=(i,)) for i in range(6)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    assert completed_queries == 60
    assert len(pool._available) == 3
    print("  ✓ 6 concurrent threads executed 60 queries through 3-connection pool.")
    print("✅ Python Capstone 03 Connection Pool PASSED!")

if __name__ == "__main__":
    test_connection_pool()
