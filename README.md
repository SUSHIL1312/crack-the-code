# 🚀 Crack the Code: Zero to Hero FAANG Interview Preparation

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17%2F20%2F23-blue.svg)](https://en.cppreference.com/)
[![Python Version](https://img.shields.io/badge/Python-3.12%2B%20%7C%203.13-yellow.svg)](https://docs.python.org/3/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Build & Tests](https://img.shields.io/badge/Tests-Passing-brightgreen.svg)](Makefile)

A comprehensive, production-quality, code-first engineering platform designed to take software engineers from fundamental principles to deep internal mastery and interview readiness for tier-1 product companies and FAANG organizations:

> **Google • Amazon • Microsoft • Meta • Apple • Netflix • Uber • NVIDIA • Qualcomm • Adobe • Salesforce • Atlassian**

---

<!-- Google Search Engine Crawler & Rich Snippets Metadata -->
<script type="application/ld+json">
{
  "@context": "https://schema.org",
  "@type": "Course",
  "name": "Crack the Code: Zero to Hero FAANG Interview Preparation",
  "description": "Production-quality, code-first curriculum preparing software engineers for technical interviews at Google, Meta, Amazon, Microsoft, Apple, Netflix, and Uber. Covers Operating Systems, Computer Networks, Modern C++20/C++23, System Design, REST API Architecture, and Concurrency with hands-on labs and test suites.",
  "provider": {
    "@type": "Person",
    "name": "Sushil",
    "url": "https://github.com/SUSHIL1312"
  },
  "educationalLevel": "Advanced",
  "about": [
    "FAANG Interview Preparation",
    "Operating Systems Internals",
    "Computer Networks",
    "Modern C++20 and C++23",
    "System Design and REST API Architecture",
    "Concurrency and Multithreading",
    "Linux Systems Programming",
    "Python CPython Internals"
  ],
  "hasCourseInstance": {
    "@type": "CourseInstance",
    "courseMode": "Online",
    "courseWorkload": "PT30H"
  }
}
</script>

---

## 🎯 Learning Philosophy: Learn by Doing

This repository rejects shallow theory notes and passive reading. Every concept is taught through an uncompromising 7-stage engineering pipeline:

$$\text{Concept} \longrightarrow \text{Internal Mechanics} \longrightarrow \text{Visualization} \longrightarrow \text{Implementation} \longrightarrow \text{Experiment \& Break} \longrightarrow \text{Interview Traps} \longrightarrow \text{Advanced Challenge}$$

1. **Why It Exists**: Real-world analogy and engineering motivation.
2. **Under the Hood**: How the OS kernel, hardware memory hierarchy, or language runtime handles it.
3. **Visual Diagrams**: Memory layout, stack/heap frames, and state transition graphs.
4. **Interactive Coding Labs**: Candidate problem templates with explicit `// TODO` blocks and automated test suites (`make test`) that report PASS/FAIL.
5. **Reference Solutions**: Battle-tested, idiomatic implementations in `solutions/` with complexity and tradeoff analyses.
6. **Interview Drills**: Detailed questions, model answers, follow-ups, and common candidate traps.
7. **Advanced Challenge**: Real-world implementation problem combining multiple concepts.

---

## 📂 Repository Architecture

```
crack-the-code/
├── README.md                           # Main portal and search engine index
├── LEARNING_GUIDE.md                   # How to study, experiment, and debug
├── ROADMAP.md                          # Sequential learning progression
├── INTERVIEW_PREPARATION.md            # Fast-track 7-day, 14-day, and 30-day plans
├── CONTRIBUTING.md                     # Code quality rules and guidelines
├── RESEARCH_SOURCES.md                 # Authoritative references (OSTEP, RFCs, PEPs, C++ Core Guidelines)
├── robots.txt                          # Crawler discovery directive
├── sitemap.xml                         # Canonical search engine sitemap
│
├── dsa/                                # Data Structures & Algorithms Hub
│   ├── README.md                       # DSA overview and navigation
│   └── leetcode/                       # Submodule -> SUSHIL1312/LeetCodeCheck.git
│
├── system-design/                      # System Design Hub
│   ├── README.md                       # System Design overview
│   ├── rest-api-design/                # Production REST API Design (Idempotency, Pagination, ETags, RFC 7807)
│   ├── low-level-design/               # Submodule -> SUSHIL1312/DesignSystem.git
│   └── high-level-design/              # Distributed Systems & HLD placeholder
│
├── core-cs/                            # Core Computer Science Curriculum
│   ├── README.md                       # Core CS overview
│   ├── operating-systems/              # 14 In-depth modules + 7 capstone projects + Hands-on Labs
│   └── computer-networks/              # 13 In-depth modules + 7 capstone projects (C++ / Sockets)
│
├── languages/                          # Programming Languages Curriculum
│   ├── README.md                       # Languages overview
│   ├── modern-cpp/                     # 25 In-depth modules + 10 capstone projects (C++17/20/23)
│   └── python/                         # 23 In-depth modules + 10 capstone projects (Python 3.12+)
│
└── ai-systems/                         # AI & Agentic Architectures (Future Phase)
    └── agentic-systems/                # Placeholder
```

---

## ⚡ Quick Start

### 1. Clone with Submodules
To clone the entire repository including the integrated **LeetCodeCheck** and **DesignSystem** submodules:

```bash
git clone --recursive https://github.com/SUSHIL1312/crack-the-code.git
cd crack-the-code
```

If you already cloned without `--recursive`, initialize submodules with:
```bash
git submodule update --init --recursive
```

### 2. Environment Verification
```bash
# Verify C++ compiler (requires C++20 support)
clang++ --version || g++ --version

# Verify Python version (requires 3.12+)
python3 --version

# Run verification suite
make help
```

---

## 📚 Curriculum Highlights

### 1. [Core CS: Operating Systems](core-cs/operating-systems/)
* **Foundations**: Hardware, CPU modes, ELF binary structure, system calls vs libc, `/proc` filesystem.
* **Concurrency**: Mutual exclusion, spinlocks with `std::atomic_flag`, counting semaphores, condition variables, reader-writer locks, deadlock detection & Banker's algorithm.
* **Memory**: Multi-level paging, page replacement simulation (FIFO, LRU, Optimal), TLB, copy-on-write, free-list allocator.
* **Projects**: Mini Shell, Production Thread Pool, Bounded Queue, Slab Allocator, Epoll Server.

### 2. [Core CS: Computer Networks](core-cs/computer-networks/)
* **Protocols**: OSI & TCP/IP stack, CIDR subnetting, Ethernet, ARP, and switching.
* **Transport**: Deep TCP internals (handshake, sequence math, sliding window, Reno/Cubic congestion control, TIME_WAIT, RST), UDP, and QUIC/HTTP/3.
* **Socket Programming**: Non-blocking I/O, `epoll`, socket options (`SO_REUSEADDR`, `SO_REUSEPORT`).
* **Projects**: TCP Chat Server, RFC HTTP Server from scratch, HTTP Client, Event Loop Server.

### 3. [Modern C++](languages/modern-cpp/)
* **Deep Mechanics**: Memory layout, vtables, strict aliasing, object lifetime, and dynamic memory.
* **Resource Ownership**: Rule of Three/Five/Zero, deep move semantics (`std::move`, `std::forward`, reference collapsing), and custom smart pointers.
* **Generics**: Advanced templates, SFINAE, Concepts, fold expressions, CRTP, and template metaprogramming.
* **Concurrency**: Memory model, acquire-release semantics, cache line bouncing, and lock-free stacks.
* **Projects**: Custom Vector, Smart Pointers, Memory Pool Allocator, Concurrent LRU Cache, Async Logger.

### 4. [Python](languages/python/)
* **Object Model**: Full dunder catalog, MRO C3 linearization, slots, descriptors, and metaclasses.
* **CPython Internals**: Small object allocator (pymalloc), reference counting, generational cyclic GC, bytecode evaluation loop, and the evolving GIL (including Python 3.13 free-threaded PEP 703 and copy-and-patch JIT PEP 744).
* **Async & Concurrency**: Asyncio event loop, `TaskGroup` structured concurrency, multiprocessing with shared memory.
* **Projects**: Custom Iterators, Dynamic Plugin Framework, Memory Profiler, Async HTTP Client, CLI Tool.

### 5. [System Design: REST API Architecture](system-design/rest-api-design/)
* **Core Principles**: Roy Fielding's 6 constraints, Safe vs Idempotent HTTP methods, resource-oriented URI modeling.
* **Production Best Practices**: `Idempotency-Key` headers for payment safety, cursor/keyset pagination vs offset pagination, optimistic concurrency with `ETag` and `If-Match`, RFC 7807 Problem Details error schema, and token-bucket rate limiting.
* **Case Study & Code**: Complete E-Commerce API design and fully runnable reference server (`rest_api_server_demo.py`) with automated test suite.

---

## ⏱️ Fast-Track Interview Preparation Plans

Preparing for an interview in days or weeks? Consult [INTERVIEW_PREPARATION.md](INTERVIEW_PREPARATION.md) for tailored schedules:

* **Track A (7-Day Revision)**: Rapid high-yield refresher on object lifetimes, smart pointers, locks, virtual memory, TCP, and the GIL.
* **Track B (14-Day Intensive)**: Structured daily modules, hands-on implementations, and live coding exercises.
* **Track C (30-Day Complete Mastery)**: Deep-dive through all four subjects and capstone projects.

---

## 📊 Learning Progress Tracking

Use this checklist to track your journey through the curriculum:

### Operating Systems
- [ ] 00-computer-fundamentals
- [ ] 01-processes-and-threads
- [ ] 02-cpu-scheduling
- [ ] 03-context-switching
- [ ] 04-memory-management
- [ ] 05-virtual-memory
- [ ] 06-paging-and-segmentation
- [ ] 07-synchronization-primitives
- [ ] 08-deadlocks
- [ ] 09-concurrency-and-parallelism
- [ ] 10-linux-internals-syscalls
- [ ] 11-file-systems-and-io
- [ ] 12-ipc-and-shared-memory
- [ ] 13-advanced-os-concepts
- [ ] OS Capstone Projects

### Computer Networks
- [ ] 00-networking-fundamentals
- [ ] 01-osi-and-tcp-ip-model
- [ ] 02-ip-addressing-and-subnetting
- [ ] 03-ethernet-arp-and-switching
- [ ] 04-transport-layer
- [ ] 05-tcp-internals
- [ ] 06-udp-and-quic
- [ ] 07-sockets-programming
- [ ] 08-dns
- [ ] 09-http-and-https
- [ ] 10-tls-and-security
- [ ] 11-network-performance
- [ ] 12-network-programming
- [ ] Networks Capstone Projects

### Modern C++
- [ ] 00-setup-and-toolchain
- [ ] 01-cpp-fundamentals
- [ ] 02-memory-model-and-layout
- [ ] 03-pointers-and-references
- [ ] 04-dynamic-memory-management
- [ ] 05-classes-and-oop
- [ ] 06-constructors-and-destructors
- [ ] 07-copy-semantics
- [ ] 08-move-semantics
- [ ] 09-raii-and-resource-management
- [ ] 10-smart-pointers
- [ ] 11-stl-internals
- [ ] 12-iterators-and-algorithms
- [ ] 13-templates
- [ ] 14-template-metaprogramming
- [ ] 15-modern-cpp-features
- [ ] 16-lambda-and-functional-programming
- [ ] 17-exception-safety
- [ ] 18-concurrency-multithreading
- [ ] 19-atomics-and-memory-model
- [ ] 20-performance-and-optimization
- [ ] 21-design-patterns-in-cpp
- [ ] 22-advanced-cpp-internals
- [ ] 23-testing-debugging-and-tooling
- [ ] 24-interview-questions
- [ ] C++ Capstone Projects

### Python
- [ ] 00-setup-and-environment
- [ ] 01-python-fundamentals
- [ ] 02-built-in-data-structures
- [ ] 03-functions-and-scope
- [ ] 04-oop-and-data-model
- [ ] 05-dunder-methods
- [ ] 06-iterators-and-generators
- [ ] 07-decorators-and-closures
- [ ] 08-memory-management
- [ ] 09-python-internals
- [ ] 10-exceptions-and-context-managers
- [ ] 11-modules-and-packages
- [ ] 12-typing-and-type-system
- [ ] 13-descriptors-and-metaclasses
- [ ] 14-concurrency-threading
- [ ] 15-multiprocessing
- [ ] 16-async-await-asyncio
- [ ] 17-advanced-data-structures
- [ ] 18-functional-programming
- [ ] 19-performance-and-optimization
- [ ] 20-testing-and-debugging
- [ ] 21-python-in-production
- [ ] 22-interview-questions
- [ ] Python Capstone Projects
