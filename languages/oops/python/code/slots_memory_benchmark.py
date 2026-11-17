"""
Benchmark measuring memory consumption of standard classes (__dict__)
versus optimized classes (__slots__).
"""

import sys

class StandardPoint:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

class SlottedPoint:
    __slots__ = ('x', 'y')
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

def run_benchmark():
    std_p = StandardPoint(1.0, 2.0)
    slot_p = SlottedPoint(1.0, 2.0)

    # Standard point memory = instance size + __dict__ size
    std_instance_size = sys.getsizeof(std_p)
    std_dict_size = sys.getsizeof(std_p.__dict__)
    std_total = std_instance_size + std_dict_size

    # Slotted point memory = instance size only (no __dict__!)
    slot_total = sys.getsizeof(slot_p)

    savings_pct = ((std_total - slot_total) / std_total) * 100

    print("=== Memory Comparison (Single Instance) ===")
    print(f"StandardPoint instance:   {std_instance_size} bytes")
    print(f"StandardPoint __dict__:   {std_dict_size} bytes")
    print(f"StandardPoint TOTAL:      {std_total} bytes")
    print(f"SlottedPoint TOTAL:       {slot_total} bytes (No __dict__!)")
    print(f"Memory Reduction:         {savings_pct:.1f}% savings per instance")
    print("===========================================")

    assert not hasattr(slot_p, "__dict__"), "SlottedPoint must not have a __dict__"
    assert slot_total < std_total, "SlottedPoint must consume significantly less memory"

    # Test attribute restriction
    try:
        slot_p.z = 3.0
        assert False, "Should have raised AttributeError"
    except AttributeError:
        print("[PASS] Dynamic attribute addition blocked on __slots__ instance.")

    print(f"[PASS] Slotted memory benchmark passed: {savings_pct:.1f}% memory reduction verified.")

if __name__ == "__main__":
    run_benchmark()
