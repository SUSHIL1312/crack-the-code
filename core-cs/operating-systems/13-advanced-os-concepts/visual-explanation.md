# Visual Explanation: Advanced OS Concepts

Visual architectures for Containers, Cgroups v2, and NUMA memory topologies.

---

## 1. Container vs Virtual Machine Architecture

```
         VIRTUAL MACHINE (VM)                          CONTAINER
+-------------------------------------+   +-------------------------------------+
| App A         | App B               |   | App A         | App B               |
+---------------+---------------------+   +---------------+---------------------+
| Guest OS A    | Guest OS B          |   | Libs / Bin    | Libs / Bin          |
| (Full Kernel) | (Full Kernel)       |   +---------------+---------------------+
+-------------------------------------+   | Namespaces | Cgroups | Seccomp      |
| Hypervisor (KVM / Xen / ESXi)       |   +-------------------------------------+
+-------------------------------------+   | Host Linux Kernel (Ring 0)          |
| Host OS (Linux / Bare Metal)        |   +-------------------------------------+
+-------------------------------------+   | Physical Hardware                   |
| Physical Hardware                   |   +-------------------------------------+
+-------------------------------------+

Startup: 20-60 seconds                     Startup: 50-200 milliseconds
Memory Overhead: 1-2 GB per VM             Memory Overhead: < 10 MB per Container
System Calls: Emulated / Dual-Ring         System Calls: Direct Native Kernel Execution
```

---

## 2. Linux Cgroups v2 Unified Hierarchy

```
                          /sys/fs/cgroup (Root)
                         /          |          \
                        /           |           \
           system.slice        user.slice      kubepods.slice
          (System Daemons)      (User Sessions)  (Kubernetes Pods)
                                                  /             \
                                       burstable.slice      guaranteed.slice
                                             /                      \
                                      pod-uuid-1                   pod-uuid-2
                                      ├── cpu.max                  ├── cpu.max
                                      ├── memory.max               ├── memory.max
                                      └── pids.max                 └── pids.max
```

In cgroups v2, an internal node cannot contain both processes and child cgroups (no internal processes rule), guaranteeing unambiguous hierarchical resource budgeting.

---

## 3. NUMA Topology: Dual-Socket Server

```
+--------------------------------+         +--------------------------------+
|         NUMA NODE 0            |         |         NUMA NODE 1            |
|                                |         |                                |
|  [ CPU Cores 0 - 31 ]          |         |  [ CPU Cores 32 - 63 ]         |
|  [ L1 / L2 Caches ]            |         |  [ L1 / L2 Caches ]            |
|  [ L3 Cache (64 MB) ]          |         |  [ L3 Cache (64 MB) ]          |
|               |                |         |               |                |
|       Local Memory Bus         |         |       Local Memory Bus         |
|               v                |         |               v                |
|   [ DDR5 RAM Bank: 128 GB ]    |         |   [ DDR5 RAM Bank: 128 GB ]    |
|   Latency: ~70 ns              |         |   Latency: ~70 ns              |
+--------------------------------+         +--------------------------------+
                 \                                 /
                  \   Interconnect (UPI/Infinity) /
                   <=============================>
                     Latency: ~160 - 220 ns (Slow!)
```
When Core 0 reads from DDR5 RAM on Node 1, memory traffic must cross the interconnect, causing latency to more than double.
