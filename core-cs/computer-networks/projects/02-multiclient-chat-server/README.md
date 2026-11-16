# Capstone Project 02: Multi-Client Chat Server with Broadcast

---

## 🎯 Overview

Building a scalable multi-client chat server requires managing concurrent client lifecycles, maintaining thread-safe client registries, and safely broadcasting messages across connections without deadlock.

This project implements:
1. **Thread-Safe Client Registry**: Protected by a reader-writer lock (`std::shared_mutex`), permitting concurrent reads while synchronizing client joins and departures.
2. **Deadlock-Free Message Broadcasting**: Dispatches incoming messages to all connected peers using buffered outbound queues.
3. **Automated Concurrency Harness**: Simulates multiple clients joining simultaneously, broadcasting chat messages concurrently, and verifying broadcast delivery across all peer nodes.

---

## 🚀 Build and Run

```bash
make
make run
```
