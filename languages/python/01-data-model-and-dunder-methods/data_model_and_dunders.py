#!/usr/bin/env python3
"""
data_model_and_dunders.py

Verifying Python Data Model, Pass-by-Sharing, and Dunder Mechanics.
"""

class Point:
    def __init__(self, x: int, y: int):
        self._x = x
        self._y = y

    def __repr__(self):
        return f"Point({self._x}, {self._y})"

    def __eq__(self, other):
        if not isinstance(other, Point):
            return False
        return self._x == other._x and self._y == other._y

    def __hash__(self):
        # Hash must be consistent with __eq__
        return hash((self._x, self._y))

class CustomSequence:
    def __init__(self, items):
        self._items = list(items)

    def __len__(self):
        return len(self._items)

    def __getitem__(self, idx):
        return self._items[idx]

def append_to_list(val, lst=None):
    if lst is None:
        lst = []
    lst.append(val)
    return lst

def run_tests():
    print("[TEST] Testing Python Data Model & Dunders...")

    # 1. Mutable default argument trap prevention
    l1 = append_to_list(10)
    l2 = append_to_list(20)
    assert l1 == [10]
    assert l2 == [20]
    print("  ✓ Sentinel default argument prevents cross-call mutation.")

    # 2. Hashability & Equality in Dictionaries
    p1 = Point(3, 4)
    p2 = Point(3, 4)
    assert p1 == p2
    assert hash(p1) == hash(p2)

    point_map = {p1: "Waypoint Alpha"}
    assert point_map[p2] == "Waypoint Alpha" # p2 finds p1's entry!
    print("  ✓ __eq__ and __hash__ dictionary lookup verified.")

    # 3. Custom Sequence Iteration via __getitem__
    seq = CustomSequence([100, 200, 300])
    assert len(seq) == 3
    assert seq[1] == 200
    # Python automatically derives for-loop iteration from __getitem__!
    collected = [x for x in seq]
    assert collected == [100, 200, 300]
    print("  ✓ __getitem__ sequence protocol verified.")

    print("✅ Python Module 01 Data Model PASSED!")

if __name__ == "__main__":
    run_tests()
