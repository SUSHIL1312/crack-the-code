#!/usr/bin/env python3
"""
lru_cache.py

Production-Quality Custom LRU Cache Decorator from Scratch:
- O(1) get and put using Doubly Linked List + Hash Map
- No usage of functools.lru_cache or collections.OrderedDict
- Automated verification test suite
"""

import functools

class Node:
    def __init__(self, key, value):
        self.key = key
        self.value = value
        self.prev = None
        self.next = None

class LRUCache:
    def __init__(self, capacity: int):
        self.capacity = capacity
        self.map = {}
        # Sentinel dummy head and tail nodes
        self.head = Node(None, None)
        self.tail = Node(None, None)
        self.head.next = self.tail
        self.tail.prev = self.head

    def _remove(self, node: Node):
        prev_node = node.prev
        next_node = node.next
        prev_node.next = next_node
        next_node.prev = prev_node

    def _add_to_front(self, node: Node):
        node.next = self.head.next
        node.prev = self.head
        self.head.next.prev = node
        self.head.next = node

    def get(self, key):
        if key not in self.map:
            return None
        node = self.map[key]
        self._remove(node)
        self._add_to_front(node)
        return node.value

    def put(self, key, value):
        if key in self.map:
            node = self.map[key]
            node.value = value
            self._remove(node)
            self._add_to_front(node)
            return

        if len(self.map) >= self.capacity:
            # Evict LRU (node right before tail)
            lru_node = self.tail.prev
            self._remove(lru_node)
            del self.map[lru_node.key]

        new_node = Node(key, value)
        self._add_to_front(new_node)
        self.map[key] = new_node

def custom_lru_cache(maxsize: int = 128):
    cache = LRUCache(maxsize)
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args):
            cached = cache.get(args)
            if cached is not None:
                return cached
            result = func(*args)
            cache.put(args, result)
            return result
        return wrapper
    return decorator

def test_lru_cache():
    print("[TEST] Custom LRU Cache Decorator...")

    call_count = 0

    @custom_lru_cache(maxsize=2)
    def compute_expensive(x: int) -> int:
        nonlocal call_count
        call_count += 1
        return x * x * x

    assert compute_expensive(2) == 8
    assert call_count == 1

    # Cache hit
    assert compute_expensive(2) == 8
    assert call_count == 1

    assert compute_expensive(3) == 27
    assert call_count == 2

    # Put 4: should evict 2 (LRU)
    assert compute_expensive(4) == 64
    assert call_count == 3

    # Calling 2 again triggers recomputation (was evicted)
    assert compute_expensive(2) == 8
    assert call_count == 4

    print("  ✓ O(1) Doubly linked list LRU cache and eviction verified.")
    print("✅ Python Capstone 02 LRU Cache Decorator PASSED!")

if __name__ == "__main__":
    test_lru_cache()
