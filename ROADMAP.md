# Engineering Roadmap: Zero to Hero FAANG Curriculum

This roadmap outlines the systematic path to complete mastery across Core Computer Science, Modern C++, and Python. Follow the tracks sequentially or prioritize according to your interview timeline.

---

## 🗺️ High-Level Learning Flow

```mermaid
graph TD
    subgraph Stage 1: Computer Systems & Toolchain
        A1[OS Fundamentals & Memory Layout] --> A2[Processes, Threads & CPU Scheduling]
        A2 --> A3[Modern C++ / Python Fundamentals]
    end

    subgraph Stage 2: Concurrency & Runtimes
        B1[Synchronization Primitives & Deadlocks] --> B2[C++ Concurrency & Atomics]
        B1 --> B3[Python GIL, Threading & Asyncio]
    end

    subgraph Stage 3: Systems I/O & Networking
        C1[Linux Syscalls, VFS & IPC] --> C2[Computer Networks: Transport & Sockets]
        C2 --> C3[HTTP, TLS & Network Programming]
    end

    subgraph Stage 4: Capstone Engineering & Interview Ready
        D1[Capstone Systems Projects] --> D2[FAANG Question Banks & Mock Drills]
        D2 --> D3[Offer Ready]
    end

    Stage 1 --> Stage 2
    Stage 2 --> Stage 3
    Stage 3 --> Stage 4
```

---

## 🚦 Progression by Experience Level

### Level 1: Fundamentals (Beginner)
* **Goal**: Write bug-free code, understand how programs execute, manage basic memory and data structures.
* **Core CS**:
  * [OS 00: Computer Fundamentals](core-cs/operating-systems/00-computer-fundamentals)
  * [OS 01: Processes & Threads](core-cs/operating-systems/01-processes-and-threads)
  * [Networks 00: Fundamentals](core-cs/computer-networks/00-networking-fundamentals)
  * [Networks 01: OSI & TCP/IP Model](core-cs/computer-networks/01-osi-and-tcp-ip-model)
* **Languages**:
  * [Modern C++ 00-05: Toolchain to OOP](languages/modern-cpp)
  * [Python 00-04: Fundamentals to OOP & Data Model](languages/python)

### Level 2: Engineering Craftsmanship (Intermediate)
* **Goal**: Design robust systems, manage resource lifetimes with RAII, understand virtual memory, handle synchronization.
* **Core CS**:
  * [OS 02-06: Scheduling, Memory Management, Virtual Memory, Paging](core-cs/operating-systems)
  * [OS 07-08: Synchronization Primitives & Deadlocks](core-cs/operating-systems)
  * [Networks 02-06: IP Subnetting, Ethernet/ARP, TCP Internals, UDP & QUIC](core-cs/computer-networks)
* **Languages**:
  * [Modern C++ 06-12: Special Members, Copy/Move, RAII, Smart Pointers, STL Internals](languages/modern-cpp)
  * [Python 05-11: Dunder Methods, Iterators/Generators, Decorators, Context Managers](languages/python)

### Level 3: Deep Internals & Scalability (Advanced)
* **Goal**: Understand runtime internals, memory barriers, non-blocking I/O multiplexing, metaprogramming, performance profiling.
* **Core CS**:
  * [OS 09-13: Multicore Concurrency, Linux Syscalls, VFS, IPC, Advanced OS](core-cs/operating-systems)
  * [Networks 07-12: Socket Programming, DNS, HTTP/HTTPS, TLS, Network Performance](core-cs/computer-networks)
* **Languages**:
  * [Modern C++ 13-23: Templates, Metaprogramming, Atomics & Memory Model, Optimization, Internals](languages/modern-cpp)
  * [Python 12-21: Type System, Descriptors, Metaclasses, GIL & Multiprocessing, Asyncio, Internals](languages/python)

### Level 4: Interview Ready (Staff / FAANG Level)
* **Goal**: Answer conceptual, scenario-based, debugging, and live coding questions with deep precision and zero hesitation.
* **Deliverables**:
  * Execute fast-track study tracks in [INTERVIEW_PREPARATION.md](INTERVIEW_PREPARATION.md).
  * Build and explain the capstone projects in [Operating Systems](core-cs/operating-systems/projects), [Computer Networks](core-cs/computer-networks/projects), [Modern C++](languages/modern-cpp/projects), and [Python](languages/python/projects).
  * Continuously solve algorithmic challenges in [dsa/leetcode](dsa/leetcode) and review design patterns in [system-design/low-level-design](system-design/low-level-design).
