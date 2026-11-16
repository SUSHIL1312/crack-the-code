#include "process_monitor.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <libproc.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif

ProcessMonitor::ProcessMonitor() = default;
ProcessMonitor::~ProcessMonitor() = default;

SystemMemoryInfo ProcessMonitor::get_system_memory() {
    SystemMemoryInfo mem{0, 0, 0};

#if defined(__linux__)
    std::ifstream file("/proc/meminfo");
    std::string key;
    uint64_t val;
    std::string unit;

    while (file >> key >> val >> unit) {
        if (key == "MemTotal:") mem.total_kb = val;
        else if (key == "MemFree:") mem.free_kb = val;
        else if (key == "MemAvailable:") mem.available_kb = val;
    }
#elif defined(__APPLE__)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    int64_t physical_memory = 0;
    size_t length = sizeof(physical_memory);
    if (sysctl(mib, 2, &physical_memory, &length, nullptr, 0) == 0) {
        mem.total_kb = physical_memory / 1024;
    }
    // Approximating free and available via host statistics
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vm_stat;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
        mem.free_kb = (vm_stat.free_count * 4096) / 1024;
        mem.available_kb = ((vm_stat.free_count + vm_stat.inactive_count) * 4096) / 1024;
    }
#endif
    return mem;
}

ProcessInfo ProcessMonitor::get_process_info(int pid) {
    ProcessInfo info{};
    info.pid = pid;
    info.name = "unknown";
    info.state = '?';
    info.vms_kb = 0;
    info.rss_kb = 0;
    info.num_threads = 1;
    info.utime_ticks = 0;
    info.stime_ticks = 0;
    info.cpu_percent = 0.0;

#if defined(__linux__)
    // 1. Read /proc/[pid]/status
    std::ifstream status_file("/proc/" + std::to_string(pid) + "/status");
    if (status_file.is_open()) {
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.rfind("Name:", 0) == 0) {
                info.name = line.substr(6);
                // trim whitespace
                info.name.erase(0, info.name.find_first_not_of(" \t"));
            } else if (line.rfind("State:", 0) == 0) {
                size_t idx = line.find_first_not_of(" \t", 6);
                if (idx != std::string::npos) info.state = line[idx];
            } else if (line.rfind("VmSize:", 0) == 0) {
                std::stringstream ss(line.substr(7));
                ss >> info.vms_kb;
            } else if (line.rfind("VmRSS:", 0) == 0) {
                std::stringstream ss(line.substr(6));
                ss >> info.rss_kb;
            } else if (line.rfind("Threads:", 0) == 0) {
                std::stringstream ss(line.substr(8));
                ss >> info.num_threads;
            }
        }
    }

    // 2. Read /proc/[pid]/stat for CPU ticks
    std::ifstream stat_file("/proc/" + std::to_string(pid) + "/stat");
    if (stat_file.is_open()) {
        std::string unused;
        // Fields: pid (1), comm (2), state (3), ppid (4), pgrp (5), session (6), tty_nr (7),
        // tpgid (8), flags (9), minflt (10), cminflt (11), majflt (12), cmajflt (13),
        // utime (14), stime (15)
        for (int i = 1; i <= 13; ++i) stat_file >> unused;
        stat_file >> info.utime_ticks >> info.stime_ticks;
    }

#elif defined(__APPLE__)
    struct proc_taskinfo pti;
    int ret = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti));
    if (ret == sizeof(pti)) {
        info.vms_kb = pti.pti_virtual_size / 1024;
        info.rss_kb = pti.pti_resident_size / 1024;
        info.num_threads = pti.pti_threadnum;
        info.utime_ticks = pti.pti_total_user;
        info.stime_ticks = pti.pti_total_system;
        info.state = 'R';
    }

    char name_buf[256] = {0};
    if (proc_name(pid, name_buf, sizeof(name_buf)) > 0) {
        info.name = name_buf;
    }
#endif

    return info;
}

std::vector<ProcessInfo> ProcessMonitor::get_all_processes(size_t limit) {
    std::vector<ProcessInfo> processes;

#if defined(__linux__)
    DIR* dir = opendir("/proc");
    if (!dir) return processes;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            char* endptr = nullptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0' && pid > 0) {
                processes.push_back(get_process_info(static_cast<int>(pid)));
            }
        }
    }
    closedir(dir);

#elif defined(__APPLE__)
    int num_pids = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    if (num_pids > 0) {
        std::vector<pid_t> pids(num_pids);
        num_pids = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), sizeof(pid_t) * pids.size());
        for (int i = 0; i < num_pids; ++i) {
            if (pids[i] > 0) {
                ProcessInfo p = get_process_info(pids[i]);
                if (!p.name.empty() && p.name != "unknown") {
                    processes.push_back(p);
                }
            }
        }
    }
#endif

    // Sort by RSS descending
    std::sort(processes.begin(), processes.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.rss_kb > b.rss_kb;
    });

    if (processes.size() > limit) {
        processes.resize(limit);
    }
    return processes;
}
