#!/usr/bin/env python3
"""
cpython_gc_internals.py

Inspecting sys.getrefcount, generational cyclic garbage collection, and weak references.
"""

import sys
import gc
import weakref

class CircularNode:
    def __init__(self, name):
        self.name = name
        self.peer = None

    def __del__(self):
        # Destructor invoked when collected
        pass

def test_reference_counting():
    print("[TEST] CPython Reference Counting Mechanics...")

    obj = [1, 2, 3]
    # sys.getrefcount includes the temporary reference passed to the function itself (+1)
    base_refs = sys.getrefcount(obj)
    
    alias = obj
    assert sys.getrefcount(obj) == base_refs + 1

    del alias
    assert sys.getrefcount(obj) == base_refs
    print(f"  ✓ Reference count accurately incremented and decremented (Base: {base_refs}).")

def test_cyclic_gc():
    print("[TEST] Generational Cyclic Garbage Collection...")

    gc.disable() # Temporarily disable automatic GC
    gc.collect()

    node1 = CircularNode("Node1")
    node2 = CircularNode("Node2")
    node1.peer = node2
    node2.peer = node1

    # Create weak references to monitor their destruction
    w1 = weakref.ref(node1)
    w2 = weakref.ref(node2)

    del node1
    del node2

    # Despite 'del', reference cycle keeps objects alive in memory!
    assert w1() is not None
    assert w2() is not None
    print("  ✓ Reference cycle prevented immediate reference-counting deallocation.")

    # Explicitly invoke cyclic collector
    reclaimed = gc.collect()
    assert reclaimed > 0
    assert w1() is None
    assert w2() is None
    print(f"  ✓ Cyclic GC identified and reclaimed {reclaimed} cycle-locked objects.")

    gc.enable()

if __name__ == "__main__":
    test_reference_counting()
    test_cyclic_gc()
    print("✅ Python Module 02 CPython GC PASSED!")
