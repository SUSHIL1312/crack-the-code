/**
 * @file proc_parser_demo.cpp
 * @brief Reads and parses Linux /proc virtual filesystem to inspect runtime process metrics:
 *        PID, PPID, State, Virtual Memory (VmSize), and Resident Set Size (VmRSS).
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic proc_parser_demo.cpp -o proc_parser
 *   ./proc_parser
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unistd.h>

void parse_proc_status() {
    std::string path = "/proc/self/status";
    std::ifstream status_file(path);

    if (!status_file.is_open()) {
        std::cout << "[Note] /proc/self/status not found (running on macOS / BSD).\n";
        std::cout << "Current Process Info via POSIX APIs:\n";
        std::cout << "  PID:  " << getpid() << "\n";
        std::cout << "  PPID: " << getppid() << "\n";
        std::cout << "On Linux, /proc/self/status provides detailed kernel memory metrics:\n"
                  << "  Name, State, VmSize, VmRSS, Threads, Voluntary_ctxt_switches.\n";
        return;
    }

    std::unordered_map<std::string, std::string> metrics;
    std::string line;

    while (std::getline(status_file, line)) {
        auto colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            // Skip colon and leading whitespace
            size_t val_start = line.find_first_not_of(" \t", colon_pos + 1);
            std::string value = (val_start != std::string::npos) ? line.substr(val_start) : "";
            metrics[key] = value;
        }
    }

    std::cout << "================ LINUX /proc/self/status METRICS ================\n";
    std::cout << "Process Name:   " << metrics["Name"] << "\n";
    std::cout << "Process State:  " << metrics["State"] << "\n";
    std::cout << "PID:            " << metrics["Pid"] << "\n";
    std::cout << "PPID:           " << metrics["PPid"] << "\n";
    std::cout << "Threads Count:  " << metrics["Threads"] << "\n";
    std::cout << "Virtual Memory: " << metrics["VmSize"] << "\n";
    std::cout << "Resident (RAM): " << metrics["VmRSS"] << "\n";
    std::cout << "Voluntary Switches:   " << metrics["voluntary_ctxt_switches"] << "\n";
    std::cout << "Non-voluntary Switches: " << metrics["nonvoluntary_ctxt_switches"] << "\n";
    std::cout << "=================================================================\n";
}

int main() {
    parse_proc_status();
    return 0;
}
