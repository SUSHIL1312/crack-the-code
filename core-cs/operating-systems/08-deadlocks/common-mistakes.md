# Common Mistakes & Traps: Deadlocks

---

## 1. The Self-Deadlock on a Non-Recursive Mutex

### The Bug:
```cpp
class DatabaseConnection {
public:
    void execute_query(const std::string& sql) {
        std::lock_guard<std::mutex> lock(mutex_);
        // ... do work ...
    }

    void execute_transaction(const std::vector<std::string>& queries) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& q : queries) {
            execute_query(q); // ❌ DEADLOCK! Same thread attempts to re-lock 'mutex_'!
        }
    }

private:
    std::mutex mutex_;
};
```

### Why It Fails:
`std::mutex` in C++ is **non-recursive** (non-reentrant). If a thread already holds `mutex_` and attempts to acquire it a second time, it deadlocks itself permanently!

### The Fixes:
1. Prefer refactoring into private unlocked helper methods:
   ```cpp
   void execute_query(const std::string& sql) {
       std::lock_guard<std::mutex> lock(mutex_);
       execute_query_unlocked(sql);
   }
   void execute_transaction(const std::vector<std::string>& queries) {
       std::lock_guard<std::mutex> lock(mutex_);
       for (const auto& q : queries) execute_query_unlocked(q);
   }
   ```
2. Or use `std::recursive_mutex` (though it introduces extra overhead and often masks architectural design smells).

---

## 2. Distributed Microservice Deadlocks

### The Scenario:
- Service A receives Request 1, acquires local Lock A, and sends an RPC to Service B.
- Simultaneously, Service B receives Request 2, acquires local Lock B, and sends an RPC to Service A.
- Service A stalls waiting for Service B; Service B stalls waiting for Service A.
- Neither service knows a cycle exists because their lock managers are on separate machines!

### The Mitigation:
- In distributed architectures, never hold local locks across remote network RPCs.
- Always configure strict, short **RPC timeouts** (e.g. 500 ms) so stalled remote calls abort and release locks rather than freezing indefinitely.
