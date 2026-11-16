# System Design Hub

System design is divided into two disciplines: **Low-Level Design (LLD)** (Object-Oriented Design, Design Patterns, Clean Architecture) and **High-Level Design (HLD)** (Distributed Systems, Scalability, Storage, Availability).

---

## 📂 Architecture

```
system-design/
├── rest-api-design/       # Production REST API Design (Idempotency, Pagination, ETags, RFC 7807)
├── low-level-design/      # Git submodule -> SUSHIL1312/DesignSystem.git
└── high-level-design/     # Distributed Systems & System Architecture (Placeholder)
```

---

## 1. REST API Design (Production-Grade)
* **Path**: [`rest-api-design/`](./rest-api-design)
* **Coverage**: Complete guide to production REST API architecture, HTTP verbs (Safety vs Idempotency), status codes, Idempotency-Key headers, cursor-based pagination, RFC 7807 Problem Details error formats, optimistic concurrency with ETags, and an end-to-end runnable reference server.

---

## 2. Low-Level Design (LLD) / Design Patterns
* **Integrated Repository**: [`SUSHIL1312/DesignSystem`](https://github.com/SUSHIL1312/DesignSystem.git)
* **Local Submodule Path**: [`low-level-design/`](./low-level-design)
* **Focus**: Gang of Four (GoF) design patterns, SOLID principles, class diagrams, schema design, and modular software implementation.
* **Guideline**: GoF design pattern implementations and object-oriented case studies are maintained directly in `DesignSystem` to avoid duplication. See the Modern C++ and Python modules in this repository for language-specific concurrency patterns, RAII wrappers, and descriptors.

---

## 2. High-Level Design (HLD)
* **Status**: Placeholder for future distributed systems curriculum.
* **Path**: [`high-level-design/`](./high-level-design)
* **Future Coverage**: Microservices, Message Brokers (Kafka, RabbitMQ), Distributed Caching (Redis), Database Sharding & Replication, CAP theorem, Consensus (Raft/Paxos), and large-scale architectural case studies.

---

## 🛠️ Submodule Commands

To initialize or update the LLD submodule:
```bash
git submodule update --init --recursive system-design/low-level-design
```
