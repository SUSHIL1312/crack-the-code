#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct ProcessInfo {
    int pid;
    std::string name;
    char state;
    uint64_t vms_kb;
    uint64_t rss_kb;
    uint32_t num_threads;
    uint64_t utime_ticks;
    uint64_t stime_ticks;
    double cpu_percent;
};

struct SystemMemoryInfo {
    uint64_t total_kb;
    uint64_t free_kb;
    uint64_t available_kb;
};

class ProcessMonitor {
public:
    ProcessMonitor();
    ~ProcessMonitor();

    SystemMemoryInfo get_system_memory();
    std::vector<ProcessInfo> get_all_processes(size_t limit = 10);
    ProcessInfo get_process_info(int pid);

private:
#if defined(__linux__)
    uint64_t get_total_system_cpu_ticks();
    uint64_t prev_total_ticks_{0};
#endif
};
