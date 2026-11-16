# High-Level Design (HLD) & Distributed Systems

> [!NOTE]
> **Curriculum Status: Future Phase**
> This section is reserved as a placeholder for comprehensive High-Level System Design and Distributed Systems preparation. In the current phase, focus is directed toward Core Computer Science (OS, Networking) and Programming Languages (Modern C++, Python).

---

## 📚 Recommended Foundational References

For candidates preparing for distributed systems interviews ahead of the upcoming curriculum release, the following primary sources and industry engineering materials are recommended:

### 1. Foundational Texts & Papers
* **Designing Data-Intensive Applications (DDIA)** by Martin Kleppmann — The gold standard for distributed storage, replication, partitioning, transactions, and consensus.
* **The Google File System (GFS)** (Ghemawat et al., 2003) — Architecture of distributed storage.
* **MapReduce: Simplified Data Processing on Large Clusters** (Dean & Ghemawat, 2004).
* **Dynamo: Amazon's Highly Available Key-value Store** (DeCandia et al., 2007) — Masterless replication, vector clocks, consistent hashing, and sloppy quorums.
* **In Search of an Understandable Consensus Algorithm (Raft)** (Ongaro & Ousterhout, 2014).

### 2. High-Level Concepts to Master
* **Scalability & Load Balancing**: Horizontal vs vertical scaling, DNS round-robin, L4 vs L7 load balancers, consistent hashing.
* **Caching**: Cache-aside, read-through, write-through, write-behind, eviction policies (LRU, LFU), cache stampede prevention.
* **Databases**: Relational vs NoSQL, ACID vs BASE, replication topologies, sharding strategies, indexing (B-Trees vs LSM-Trees).
* **Asynchronous Communication**: Message queues (Kafka, RabbitMQ), event-driven architectures, backpressure, idempotency.
* **Reliability & Fault Tolerance**: Heartbeats, circuit breakers, rate limiters, retries with exponential backoff and jitter.
