#include "process_monitor.hpp"
#include <iostream>
#include <iomanip>
#include <unistd.h>

int main() {
    std::cout << "================ LINUX / POSIX PROCESS MONITOR ================\n";

    ProcessMonitor monitor;

    // 1. System Memory Overview
    SystemMemoryInfo mem = monitor.get_system_memory();
    std::cout << "System Memory Summary:\n";
    std::cout << "  Total RAM:     " << mem.total_kb / 1024 << " MB (" << mem.total_kb << " KB)\n";
    std::cout << "  Free RAM:      " << mem.free_kb / 1024 << " MB\n";
    std::cout << "  Available RAM: " << mem.available_kb / 1024 << " MB\n\n";

    // 2. Current Process Self Inspection
    int my_pid = getpid();
    ProcessInfo self_info = monitor.get_process_info(my_pid);
    std::cout << "Self Process Inspection (PID " << my_pid << "):\n";
    std::cout << "  Name:          " << self_info.name << "\n";
    std::cout << "  State:         " << self_info.state << "\n";
    std::cout << "  Resident (RSS):" << self_info.rss_kb << " KB (" << self_info.rss_kb / 1024.0 << " MB)\n";
    std::cout << "  Virtual (VMS): " << self_info.vms_kb << " KB (" << self_info.vms_kb / 1024.0 << " MB)\n";
    std::cout << "  Threads:       " << self_info.num_threads << "\n\n";

    // 3. Top Processes by RSS Memory
    std::cout << "Top Active Processes by RSS Memory:\n";
    std::cout << std::left
              << std::setw(8)  << "PID"
              << std::setw(25) << "COMMAND"
              << std::setw(12) << "RSS (MB)"
              << std::setw(12) << "VMS (MB)"
              << std::setw(10) << "THREADS"
              << "\n";
    std::cout << std::string(67, '-') << "\n";

    auto top_procs = monitor.get_all_processes(10);
    for (const auto& p : top_procs) {
        std::cout << std::left
                  << std::setw(8)  << p.pid
                  << std::setw(25) << p.name.substr(0, 24)
                  << std::setw(12) << std::fixed << std::setprecision(1) << (p.rss_kb / 1024.0)
                  << std::setw(12) << std::fixed << std::setprecision(1) << (p.vms_kb / 1024.0)
                  << std::setw(10) << p.num_threads
                  << "\n";
    }

    std::cout << "=================================================================\n";
    return 0;
}
