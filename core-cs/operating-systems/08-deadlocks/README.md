# Module 08: Deadlocks & Resource Allocation

Deadlocks occur when a set of concurrent processes are permanently blocked because each process holds a resource and waits for another resource held by someone else in the set. Understanding the four Coffman conditions, prevention strategies, and avoidance algorithms is a core requirement for systems reliability.

---

## 📑 Module Contents

1. [Theory & Coffman Conditions](theory.md)
   * The Four Necessary and Sufficient Coffman Conditions.
   * Deadlock vs Livelock vs Starvation.
   * Four Paradigms: Prevention, Avoidance, Detection & Recovery, Ostrich Algorithm.
   * Breaking Circular Wait through Global Resource Hierarchies.
2. [Internal Working](internal-working.md)
   * Resource Allocation Graphs (RAG) and Cycle Detection.
   * Dijkstra's Banker's Algorithm: Safe vs Unsafe States, Allocation Vectors.
   * Deadlock Detection via Tarjan's Strongly Connected Components / DFS cycle check.
3. [Visual Explanations](visual-explanation.md)
   * Resource Allocation Graph cycle diagrams.
   * Lock ordering hierarchy breakdown.
4. [Examples & Code](examples/)
   * `deadlock_demo_and_fix.cpp`: Reproduces real multithreaded lock-inversion deadlocks and fixes them via Lock Ordering and Modern C++ `std::scoped_lock`.
   * `bankers_algorithm.cpp`: Complete, robust C++ implementation of Dijkstra's Banker's Algorithm verifying system safety and granting resource requests.
5. [Exercises & Solutions](exercises/exercises.md)
   * Tracing Banker's Algorithm allocation matrices, proving cycle detection.
6. [Interview Questions & Deep Dives](interview-questions.md)
   * Essential FAANG questions: Coffman conditions, Deadlock vs Livelock, Real-world prevention in large scale distributed systems.
7. [Common Mistakes](common-mistakes.md)
   * Conflating Unsafe State with Deadlocked State, circular lock dependencies across microservices.
