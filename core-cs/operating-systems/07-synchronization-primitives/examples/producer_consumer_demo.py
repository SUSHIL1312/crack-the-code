"""
producer_consumer_demo.py - Producer-Consumer (Bounded Buffer) in Python

Demonstrates two canonical synchronization patterns:
  1. Condition Variables (threading.Condition with wait() and notify())
  2. Counting Semaphores (threading.Semaphore with acquire() and release())

Run:
  python3 producer_consumer_demo.py
"""

import collections
import threading
import time


class BoundedBufferCondVar:
    """Bounded Buffer implemented using threading.Condition and while-loop predicate."""

    def __init__(self, capacity: int):
        self.capacity = capacity
        self.queue = collections.deque()
        self.lock = threading.Lock()
        self.condition = threading.Condition(self.lock)
        self.is_shutdown = False

    def push(self, item) -> bool:
        with self.condition:
            # Step 1: Wait while buffer is full
            # ALWAYS use 'while' to defend against spurious wakeups & race conditions!
            while len(self.queue) >= self.capacity and not self.is_shutdown:
                self.condition.wait()

            if self.is_shutdown:
                return False

            self.queue.append(item)

            # Step 2: Signal waiting consumers that a new item is available
            self.condition.notify()
            return True

    def pop(self):
        with self.condition:
            # Step 1: Wait while buffer is empty
            while len(self.queue) == 0 and not self.is_shutdown:
                self.condition.wait()

            if len(self.queue) == 0 and self.is_shutdown:
                return None

            item = self.queue.popleft()

            # Step 2: Signal waiting producers that an empty slot freed up
            self.condition.notify()
            return item

    def shutdown(self):
        with self.condition:
            self.is_shutdown = True
            # Wake up all sleeping producers and consumers so they can exit gracefully
            self.condition.notify_all()


class BoundedBufferSemaphore:
    """Bounded Buffer implemented using Dijkstra's canonical Semaphores."""

    def __init__(self, capacity: int):
        self.capacity = capacity
        self.queue = collections.deque()
        self.mutex = threading.Lock()
        self.empty_slots = threading.Semaphore(capacity)
        self.full_slots = threading.Semaphore(0)

    def push(self, item):
        # Step 1: Wait for an available empty slot (decrements empty_slots)
        # CRITICAL: Acquire semaphore BEFORE the mutex to avoid deadlock!
        self.empty_slots.acquire()

        with self.mutex:
            self.queue.append(item)

        # Step 2: Signal that a new filled slot is ready (increments full_slots)
        self.full_slots.release()

    def pop(self):
        # Step 1: Wait for a filled slot to be available (decrements full_slots)
        self.full_slots.acquire()

        with self.mutex:
            item = self.queue.popleft()

        # Step 2: Signal that an empty slot freed up (increments empty_slots)
        self.empty_slots.release()
        return item


def test_condvar_buffer():
    print("--- [1] Testing Producer-Consumer with Condition Variable ---")
    capacity = 3
    num_items = 15
    buf = BoundedBufferCondVar(capacity)
    consumed_items = []

    def producer():
        for i in range(num_items):
            buf.push(f"task-{i}")
            time.sleep(0.001)
        buf.shutdown()

    def consumer():
        while True:
            item = buf.pop()
            if item is None:
                break
            consumed_items.append(item)
            time.sleep(0.002)

    t_prod = threading.Thread(target=producer)
    t_cons = threading.Thread(target=consumer)

    t_prod.start()
    t_cons.start()
    t_prod.join()
    t_cons.join()

    assert len(consumed_items) == num_items, f"Expected {num_items}, got {len(consumed_items)}"
    assert consumed_items == [f"task-{i}" for i in range(num_items)]
    print(f"  [PASS] Condition Variable processed all {num_items} items in exact FIFO order.\n")


def test_semaphore_buffer():
    print("--- [2] Testing Producer-Consumer with Semaphores ---")
    capacity = 3
    num_producers = 3
    items_per_producer = 5
    total_items = num_producers * items_per_producer

    buf = BoundedBufferSemaphore(capacity)
    consumed_count = 0
    consumed_lock = threading.Lock()

    def producer(pid):
        for i in range(items_per_producer):
            buf.push(f"p{pid}-item{i}")
            time.sleep(0.001)

    def consumer():
        nonlocal consumed_count
        while True:
            with consumed_lock:
                if consumed_count >= total_items:
                    break
            buf.pop()
            with consumed_lock:
                consumed_count += 1
            time.sleep(0.001)

    producers = [threading.Thread(target=producer, args=(p,)) for p in range(num_producers)]
    consumers = [threading.Thread(target=consumer) for _ in range(2)]

    for t in consumers:
        t.start()
    for t in producers:
        t.start()

    for t in producers:
        t.join()
    for t in consumers:
        t.join()

    assert consumed_count == total_items
    print(f"  [PASS] Counting Semaphore processed all {total_items} items concurrently with zero deadlocks.\n")


if __name__ == "__main__":
    print("=====================================================")
    print("   Python Concurrency: Producer-Consumer Mastery     ")
    print("=====================================================\n")
    test_condvar_buffer()
    test_semaphore_buffer()
    print("All Python synchronization tests passed successfully!")
