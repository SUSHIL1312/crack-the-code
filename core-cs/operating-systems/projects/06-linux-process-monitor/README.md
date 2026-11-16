# Project 06: Linux Process & System Monitor (`top` / `htop` Engine)

A lightweight, high-performance process monitor and `/proc` parser built in C++20 that monitors CPU usage, memory utilization (RSS, Virtual Size), thread counts, and file descriptors.

---

## Architectural Highlights
- **Linux `/proc` Parser Engine**:
  - `/proc/stat`: System-wide aggregate CPU ticks across user, nice, system, idle, iowait, irq, and softirq modes.
  - `/proc/meminfo`: Total, free, available, cached, and buffer memory.
  - `/proc/[pid]/stat`: Process-specific user ticks (`utime`), kernel ticks (`stime`), priority, nice value, and start time.
  - `/proc/[pid]/status`: Process name, state (`R`, `S`, `D`, `Z`), VmSize, VmRSS, UID/GID, and threads.
  - `/proc/[pid]/fd/`: Active open file descriptors count.
- **Portability**: Native `/proc` parsing when compiled on Linux; on macOS, integrates with Darwin's `libproc` and Mach kernel `proc_pidinfo()` APIs.
- **CPU % Computation**:
  $$\text{CPU \%} = \frac{\Delta (\text{process\_utime} + \text{process\_stime})}{\Delta \text{total\_system\_cpu\_time}} \times 100\%$$

---

## Build and Run
```bash
make
./proc_monitor
```
