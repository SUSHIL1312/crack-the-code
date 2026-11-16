# Research Sources & Authoritative References Library

This repository is built upon primary sources, authoritative industry standards, peer-reviewed computer science literature, and official engineering documentation from top technology companies.

---

## 1. Operating Systems & Systems Programming

### Primary Literature & Academic Textbooks
* **Operating Systems: Three Easy Pieces (OSTEP)** — Andrea Arpaci-Dusseau and Remzi Arpaci-Dusseau (University of Wisconsin-Madison).
  * URL: [https://pages.cs.wisc.edu/~remzi/OSTEP/](https://pages.cs.wisc.edu/~remzi/OSTEP/)
  * *Relevance*: Foundation for virtualization (CPU scheduling, multi-level paging, TLB), concurrency (locks, semaphores, condition variables), and persistence (file systems, crash consistency).
* **Modern Operating Systems (4th Edition)** — Andrew S. Tanenbaum and Herbert Bos.
  * *Relevance*: Architectural principles of microkernels vs monolithic kernels, memory management algorithms, and deadlock avoidance.
* **Linux Kernel Development (3rd Edition)** — Robert Love.
  * *Relevance*: Linux process scheduler (CFS), interrupt handlers, bottom halves, virtual memory, and kernel synchronization.
* **The Linux Programming Interface (TLPI)** — Michael Kerrisk.
  * *Relevance*: Definitive guide to Linux/UNIX system calls (`fork`, `exec`, `mmap`, `epoll`, `signals`, `IPC`).

### Official Documentation & Standards
* **Linux Manual Pages (man7.org)**:
  * URL: [https://man7.org/linux/man-pages/](https://man7.org/linux/man-pages/)
  * Key Sections: Section 2 (System calls: `clone(2)`, `fork(2)`, `epoll_create(2)`, `mmap(2)`), Section 3 (Library functions), Section 7 (Overviews: `pthreads(7)`, `socket(7)`, `futex(7)`).
* **Linux Kernel Official Documentation**:
  * URL: [https://docs.kernel.org/](https://docs.kernel.org/)
  * Key Areas: CFS scheduler design, Memory Management (page allocation, slab allocator), Control Groups (cgroups v2).

---

## 2. Computer Networks & Network Programming

### Primary Guides & References
* **Beej's Guide to Network Programming (Using Internet Sockets)** — Brian "Beej Jorgensen" Hall.
  * URL: [https://beej.us/guide/bgnet/](https://beej.us/guide/bgnet/)
  * *Relevance*: Standard reference for IPv4/IPv6 socket programming, `getaddrinfo`, non-blocking I/O, `select`/`poll`.
* **Computer Networking: A Top-Down Approach (8th Edition)** — James Kurose and Keith Ross.
  * *Relevance*: Application layer down to link layer, TCP congestion control, sliding window mechanics.

### Official RFCs (Internet Engineering Task Force)
* **RFC 793 / RFC 9293**: Transmission Control Protocol (TCP) Specification.
* **RFC 768**: User Datagram Protocol (UDP).
* **RFC 1034 & RFC 1035**: Domain Names — Concepts, Facilities, and Implementation.
* **RFC 9110 & RFC 9112**: HTTP Semantics and HTTP/1.1.
* **RFC 7540**: Hypertext Transfer Protocol Version 2 (HTTP/2).
* **RFC 9000 & RFC 9114**: QUIC: A UDP-Based Multiplexed and Secure Transport & HTTP/3.
* **RFC 8446**: The Transport Layer Security (TLS) Protocol Version 1.3.

### Web Documentation
* **MDN Web Docs — HTTP Guides**:
  * URL: [https://developer.mozilla.org/en-US/docs/Web/HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)

---

## 3. Modern C++ (C++17, C++20, C++23)

### Core Language Standards & Guidelines
* **C++ Core Guidelines** — Bjarne Stroustrup and Herb Sutter.
  * URL: [https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
  * *Relevance*: Philosophy (P), Interfaces (I), Resource Management (R: RAII), Memory & Pointer Safety, Concurrency (CP).
* **cppreference.com**:
  * URL: [https://en.cppreference.com/](https://en.cppreference.com/)
  * *Relevance*: Authoritative reference for standard library headers, memory models, value categories, and standard algorithms.
* **ISO/IEC 14882 (C++ Standard Working Drafts)**:
  * URL: [https://github.com/cplusplus/draft](https://github.com/cplusplus/draft)
* **Effective Modern C++** — Scott Meyers.
  * *Relevance*: Auto deduction, rvalue references, move semantics, perfect forwarding, smart pointer design, lambda expressions.
* **C++ Concurrency in Action (2nd Edition)** — Anthony Williams.
  * *Relevance*: Multithreading, `std::atomic`, memory barriers, lock-free data structures.

---

## 4. Python & CPython Internals

### Official Documentation
* **Python 3.12 & 3.13 Official Documentation**:
  * URL: [https://docs.python.org/3/](https://docs.python.org/3/)
  * Key Sections: Python Data Model, Standard Library (`asyncio`, `multiprocessing`, `threading`, `typing`, `collections`, `gc`, `dis`).
* **CPython Source Repository**:
  * URL: [https://github.com/python/cpython](https://github.com/python/cpython)
  * Key Subsystems: `Objects/` (`tupleobject.c`, `dictobject.c`, `listobject.c`), `Python/ceval.c` (interpreter evaluation loop), `Python/pylifecycle.c`.

### Key Python Enhancement Proposals (PEPs)
* **PEP 484, 544, 695**: Type Hints, Protocols (Structural Subtyping), and Modern Type Parameter Syntax (`def func[T](...)`).
* **PEP 492 & 525**: Coroutines with `async`/`await` and Asynchronous Generators.
* **PEP 684 & 703**: Per-Interpreter GIL & Making the Global Interpreter Lock Optional in CPython (Python 3.13 free-threaded mode).
* **PEP 709**: Inlined Comprehensions.
* **PEP 744**: JIT Compilation for CPython.

---

## 5. Major Product Companies: Official Technical Interview Guides

* **Google Careers — Technical Interview Preparation**:
  * URL: [https://careers.google.com/how-we-hire/interview/](https://careers.google.com/how-we-hire/interview/)
  * Focus: Data structures, computational complexity, memory management, scalable systems, code clarity.
* **Amazon Jobs — Software Development Topics**:
  * URL: [https://www.amazon.jobs/content/en/how-we-hire/interview-prep/software-development-topics](https://www.amazon.jobs/content/en/how-we-hire/interview-prep/software-development-topics)
  * Focus: Object-oriented design, concurrency, distributed systems, operational excellence, Leadership Principles.
* **Microsoft Careers — Technical Interviewing Tips**:
  * URL: [https://careers.microsoft.com/v2/global/en/hiring-tips/technical-interviewing.html](https://careers.microsoft.com/v2/global/en/hiring-tips/technical-interviewing.html)
  * Focus: Problem-solving methodology, data structures, trade-off analysis, edge-case handling.
* **Meta Careers — Engineering Interview Prep**:
  * URL: [https://www.metacareers.com/swe-prep](https://www.metacareers.com/swe-prep)
  * Focus: Algorithmic speed, concurrency, system architecture, performance optimization.
