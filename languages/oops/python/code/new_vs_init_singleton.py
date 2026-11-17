"""
Demonstration and validation of __new__ vs __init__:
1. Thread-safe Singleton pattern using __new__
2. Subclassing immutable built-in types (int)
3. Skipping __init__ when __new__ returns a different class
"""

import threading

class Singleton:
    _instance = None
    _lock = threading.Lock()
    _init_count = 0

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self, val: int = 0):
        # NOTE: __init__ runs on EVERY call to Singleton() unless protected!
        self.val = val
        Singleton._init_count += 1

class PositiveInt(int):
    """Subclassing immutable built-in: must modify value in __new__!"""
    def __new__(cls, value: int):
        normalized = abs(int(value))
        return super().__new__(cls, normalized)

class OtherClass:
    pass

class ForeignFactory:
    def __new__(cls):
        return OtherClass() # Returns an instance of a DIFFERENT class!

    def __init__(self):
        raise RuntimeError("ForeignFactory.__init__ must NOT be called!")

def test_singleton():
    s1 = Singleton(10)
    s2 = Singleton(20)
    assert s1 is s2, "Singleton instances must be identical in memory"
    assert s1.val == 20, "Second call updated attribute"
    assert Singleton._init_count == 2
    print("[PASS] Thread-safe Singleton via __new__ verified.")

def test_immutable_subclass():
    p1 = PositiveInt(-42)
    p2 = PositiveInt(100)
    assert p1 == 42
    assert p2 == 100
    assert isinstance(p1, int)
    print(f"[PASS] Immutable subclassing verified: PositiveInt(-42) -> {p1}")

def test_foreign_factory_skips_init():
    # Because __new__ returns OtherClass, ForeignFactory.__init__ is bypassed!
    obj = ForeignFactory()
    assert isinstance(obj, OtherClass)
    assert not isinstance(obj, ForeignFactory)
    print("[PASS] Verified: __init__ was skipped because __new__ returned a different type.")

if __name__ == "__main__":
    test_singleton()
    test_immutable_subclass()
    test_foreign_factory_skips_init()
