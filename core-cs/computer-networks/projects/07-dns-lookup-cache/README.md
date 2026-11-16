# Capstone Project 07: High-Performance DNS Lookup & LRU Cache

---

## 🎯 Overview

Domain Name System (DNS) resolution is the first step in nearly every internet transaction. Production DNS resolvers (such as CoreDNS, Unbound, and BIND) achieve sub-millisecond query latencies by coupling binary wire parsers with high-concurrency, TTL-aware in-memory caches.

This project implements:
1. **RFC 1035 Binary Wire Parser**: Encodes human domains into DNS label sequences and parses DNS resource records (including pointer compression `0xC0XX`).
2. **TTL-Aware LRU Cache**: Implements an $O(1)$ Least Recently Used cache with per-record Time-to-Live (TTL) expiration.
3. **Negative Caching**: Caches non-existent domains (`NXDOMAIN`) with bounded lifetimes to mitigate cache stampedes.
4. **Automated Verification Harness**: Validates wire parsing, LRU eviction, TTL expiration, and hit/miss metrics.

---

## 🚀 Build and Run

```bash
make
make run
```
