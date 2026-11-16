# Module 00: Networking Fundamentals

## Overview
Computer networking is the foundation of distributed computing, modern cloud services, and global web infrastructure. Before diving into protocols like TCP, IP, or HTTP, engineers must understand the physical and mathematical realities governing how data travels across wires, fiber-optic cables, and wireless spectrums.

In this module, you will master the physics and mathematics of network latency, the contrast between circuit and packet switching, and the key metrics used to evaluate distributed system performance.

---

## Learning Objectives
1. **Physical Topologies**: Understand Star, Mesh, Ring, and Spine-and-Leaf architectures used in modern datacenters.
2. **Switching Paradigms**: Contrast Circuit Switching (dedicated bandwidth) with Packet Switching (statistical multiplexing).
3. **Four Sources of Packet Delay**:
   - Processing Delay ($D_{\text{proc}}$)
   - Queuing Delay ($D_{\text{queue}}$)
   - Transmission Delay ($D_{\text{trans}} = L / R$)
   - Propagation Delay ($D_{\text{prop}} = d / s$)
4. **Bandwidth vs Latency vs Jitter**: Understand why adding bandwidth cannot fix latency bounded by the speed of light in fiber.
5. **Practical Tooling**: Use `ping`, `traceroute`, and `iperf3` to diagnose network paths and bottlenecks.

---

## Curriculum Structure
- `theory.md`: Theoretical foundations, delay formulas, and network models.
- `internal-working.md`: Router packet forwarding pipelines, FIFO/Fair-Queuing buffer behavior, and propagation speed in optical fiber.
- `visual-explanation.md`: Transmission vs propagation visual analogies and delay timelines.
- `examples/`:
  - `network_delay_calculator.cpp`: Production C++ utility calculating end-to-end delay components, BDP, and fiber travel times.
- `exercises/`: Delay calculation and topology design exercises.
- `solutions/`: Step-by-step mathematical solutions.
- `interview-questions.md`: FAANG interview questions (e.g. "Why is US-to-India ping minimum ~150ms?", "Bandwidth vs throughput").
- `common-mistakes.md`: Confusing transmission delay with propagation delay, ignoring queuing delay under load.
