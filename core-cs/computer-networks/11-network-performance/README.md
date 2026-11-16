# Module 11: Network Performance & Optimization

## Overview
High-performance distributed systems, low-latency financial exchanges, and global cloud architectures require rigorous optimization of network pipelines. Raw bandwidth alone cannot solve performance bottlenecks; systems engineers must understand the mathematical constraints of the Bandwidth-Delay Product (BDP), mitigate bufferbloat, tune Linux kernel socket buffers, deploy Content Delivery Network (CDN) edge architectures, and optimize TCP handshakes.

In this module, you will master the physics of network throughput, kernel sysctl parameters (`tcp_rmem`, `tcp_wmem`, `somaxconn`), Active Queue Management (CoDel / FQ-CoDel), and build a BDP throughput analyzer in Modern C++.

---

## Learning Objectives
1. **Bandwidth-Delay Product (BDP)**: Calculate the volume of data in transit required to fully saturate high-speed links and diagnose the "Long Fat Network" (LFN) bottleneck.
2. **Bufferbloat & Active Queue Management (AQM)**:
   - Understand why oversized drop-tail router buffers cause 1-second ping spikes during downloads.
   - Master modern AQM algorithms: CoDel (Controlled Delay) and FQ-CoDel (RFC 8290).
3. **Linux Kernel TCP Tuning**:
   - Tune read and write buffers via `/proc/sys/net/ipv4/tcp_rmem` and `tcp_wmem`.
   - Configure TCP Fast Open (`tcp_fastopen`) and initial window sizing (`initcwnd`).
4. **Content Delivery Network (CDN) Architecture**:
   - Contrast Anycast BGP routing with GeoDNS routing for edge server selection.
   - Optimize cache retention with `stale-while-revalidate` and surrogate keys.
5. **Datacenter Performance Optimization**: Master Jumbo Frames (MTU 9000), TCP Segmentation Offload (TSO), and Receive Side Scaling (RSS).

---

## Curriculum Structure
- `theory.md`: Theoretical foundations of BDP, Bufferbloat, AQM algorithms, and CDN edge caching.
- `internal-working.md`: Linux autotuning memory mechanisms (`tcp_mem`, `tcp_rmem`), CoDel packet dropping state machines, and Anycast BGP convergence.
- `visual-explanation.md`: Visual diagrams of the "Long Fat Pipe", bufferbloat queue accumulation, and CDN edge termination.
- `examples/`:
  - `bdp_and_tuning_bench.cpp`: C++ performance calculator and simulator evaluating BDP, window scaling, and CoDel queue drainage.
- `exercises/`: Kernel buffer tuning calculations and CDN edge placement exercises.
- `solutions/`: Reference engineering solutions.
- `interview-questions.md`: FAANG interview questions (e.g. "How does a CDN reduce latency?", "Why does a 1 Gbps link only achieve 10 Mbps over a trans-pacific connection?").
- `common-mistakes.md`: Blindly maxing out socket buffers causing kernel OOM, setting static window sizes instead of autotuning.
