/**
 * @file scheduling_simulator.cpp
 * @brief Comprehensive CPU Scheduling Simulator comparing:
 *        - First-Come, First-Served (FCFS)
 *        - Shortest Job First (SJF)
 *        - Shortest Remaining Time First (SRTF - Preemptive SJF)
 *        - Round Robin (RR)
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic scheduling_simulator.cpp -o sched_sim
 *   ./sched_sim
 */

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

struct Process {
    int id;
    int arrival_time;
    int burst_time;
    int priority;

    // Simulation output metrics
    int remaining_time = 0;
    int start_time = -1;
    int completion_time = 0;
    int waiting_time = 0;
    int turnaround_time = 0;
    int response_time = 0;
};

struct SimulationResult {
    std::string algorithm_name;
    double avg_waiting_time = 0.0;
    double avg_turnaround_time = 0.0;
    double avg_response_time = 0.0;
    std::vector<Process> completed_processes;
};

// 1. First-Come, First-Served (FCFS)
SimulationResult simulate_fcfs(std::vector<Process> procs) {
    std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    int current_time = 0;
    for (auto& p : procs) {
        if (current_time < p.arrival_time) {
            current_time = p.arrival_time;
        }
        p.start_time = current_time;
        p.response_time = p.start_time - p.arrival_time;
        p.waiting_time = current_time - p.arrival_time;
        current_time += p.burst_time;
        p.completion_time = current_time;
        p.turnaround_time = p.completion_time - p.arrival_time;
    }

    SimulationResult res{"FCFS", 0.0, 0.0, 0.0, procs};
    for (const auto& p : procs) {
        res.avg_waiting_time += p.waiting_time;
        res.avg_turnaround_time += p.turnaround_time;
        res.avg_response_time += p.response_time;
    }
    res.avg_waiting_time /= procs.size();
    res.avg_turnaround_time /= procs.size();
    res.avg_response_time /= procs.size();
    return res;
}

// 2. Shortest Job First (Non-preemptive)
SimulationResult simulate_sjf(std::vector<Process> procs) {
    int n = static_cast<int>(procs.size());
    std::vector<bool> is_completed(n, false);
    int current_time = 0;
    int completed_count = 0;
    std::vector<Process> completed;

    while (completed_count < n) {
        int best_idx = -1;
        int min_burst = 1e9;

        for (int i = 0; i < n; ++i) {
            if (!is_completed[i] && procs[i].arrival_time <= current_time) {
                if (procs[i].burst_time < min_burst) {
                    min_burst = procs[i].burst_time;
                    best_idx = i;
                }
            }
        }

        if (best_idx == -1) {
            // CPU Idle, advance to next arriving process
            int next_arrival = 1e9;
            for (int i = 0; i < n; ++i) {
                if (!is_completed[i]) {
                    next_arrival = std::min(next_arrival, procs[i].arrival_time);
                }
            }
            current_time = next_arrival;
            continue;
        }

        auto& p = procs[best_idx];
        p.start_time = current_time;
        p.response_time = p.start_time - p.arrival_time;
        p.waiting_time = current_time - p.arrival_time;
        current_time += p.burst_time;
        p.completion_time = current_time;
        p.turnaround_time = p.completion_time - p.arrival_time;
        is_completed[best_idx] = true;
        completed_count++;
        completed.push_back(p);
    }

    SimulationResult res{"SJF (Non-preemptive)", 0.0, 0.0, 0.0, completed};
    for (const auto& p : completed) {
        res.avg_waiting_time += p.waiting_time;
        res.avg_turnaround_time += p.turnaround_time;
        res.avg_response_time += p.response_time;
    }
    res.avg_waiting_time /= n;
    res.avg_turnaround_time /= n;
    res.avg_response_time /= n;
    return res;
}

// 3. Shortest Remaining Time First (SRTF - Preemptive)
SimulationResult simulate_srtf(std::vector<Process> procs) {
    int n = static_cast<int>(procs.size());
    for (auto& p : procs) {
        p.remaining_time = p.burst_time;
    }

    int current_time = 0;
    int completed_count = 0;

    while (completed_count < n) {
        int best_idx = -1;
        int min_remaining = 1e9;

        for (int i = 0; i < n; ++i) {
            if (procs[i].arrival_time <= current_time && procs[i].remaining_time > 0) {
                if (procs[i].remaining_time < min_remaining) {
                    min_remaining = procs[i].remaining_time;
                    best_idx = i;
                }
            }
        }

        if (best_idx == -1) {
            current_time++;
            continue;
        }

        auto& p = procs[best_idx];
        if (p.start_time == -1) {
            p.start_time = current_time;
            p.response_time = p.start_time - p.arrival_time;
        }

        p.remaining_time--;
        current_time++;

        if (p.remaining_time == 0) {
            p.completion_time = current_time;
            p.turnaround_time = p.completion_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            completed_count++;
        }
    }

    SimulationResult res{"SRTF (Preemptive SJF)", 0.0, 0.0, 0.0, procs};
    for (const auto& p : procs) {
        res.avg_waiting_time += p.waiting_time;
        res.avg_turnaround_time += p.turnaround_time;
        res.avg_response_time += p.response_time;
    }
    res.avg_waiting_time /= n;
    res.avg_turnaround_time /= n;
    res.avg_response_time /= n;
    return res;
}

// 4. Round Robin (RR)
SimulationResult simulate_round_robin(std::vector<Process> procs, int quantum) {
    int n = static_cast<int>(procs.size());
    for (auto& p : procs) {
        p.remaining_time = p.burst_time;
    }

    // Sort by arrival time initially
    std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::queue<int> ready_queue;
    std::vector<bool> in_queue(n, false);
    int current_time = 0;
    int completed_count = 0;

    // Push initial processes that arrive at time 0
    for (int i = 0; i < n; ++i) {
        if (procs[i].arrival_time <= current_time) {
            ready_queue.push(i);
            in_queue[i] = true;
        }
    }

    while (completed_count < n) {
        if (ready_queue.empty()) {
            current_time++;
            for (int i = 0; i < n; ++i) {
                if (!in_queue[i] && procs[i].arrival_time <= current_time && procs[i].remaining_time > 0) {
                    ready_queue.push(i);
                    in_queue[i] = true;
                }
            }
            continue;
        }

        int idx = ready_queue.front();
        ready_queue.pop();
        auto& p = procs[idx];

        if (p.start_time == -1) {
            p.start_time = current_time;
            p.response_time = p.start_time - p.arrival_time;
        }

        int run_slice = std::min(quantum, p.remaining_time);
        p.remaining_time -= run_slice;
        current_time += run_slice;

        // Check for new arrivals while this process was executing
        for (int i = 0; i < n; ++i) {
            if (!in_queue[i] && procs[i].arrival_time <= current_time && procs[i].remaining_time > 0) {
                ready_queue.push(i);
                in_queue[i] = true;
            }
        }

        if (p.remaining_time > 0) {
            // Re-append to back of queue
            ready_queue.push(idx);
        } else {
            p.completion_time = current_time;
            p.turnaround_time = p.completion_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            completed_count++;
        }
    }

    SimulationResult res{"Round Robin (q=" + std::to_string(quantum) + ")", 0.0, 0.0, 0.0, procs};
    for (const auto& p : procs) {
        res.avg_waiting_time += p.waiting_time;
        res.avg_turnaround_time += p.turnaround_time;
        res.avg_response_time += p.response_time;
    }
    res.avg_waiting_time /= n;
    res.avg_turnaround_time /= n;
    res.avg_response_time /= n;
    return res;
}

void print_comparison(const std::vector<SimulationResult>& results) {
    std::cout << "\n================ CPU SCHEDULING SIMULATOR COMPARISON ================\n";
    std::cout << std::left << std::setw(26) << "Algorithm"
              << std::setw(18) << "Avg Waiting Time"
              << std::setw(22) << "Avg Turnaround Time"
              << std::setw(18) << "Avg Response Time" << "\n";
    std::cout << "---------------------------------------------------------------------\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(26) << r.algorithm_name
                  << std::setw(18) << std::fixed << std::setprecision(2) << r.avg_waiting_time
                  << std::setw(22) << r.avg_turnaround_time
                  << std::setw(18) << r.avg_response_time << "\n";
    }
    std::cout << "=====================================================================\n";
}

int main() {
    // Standard test workload with diverse burst times and arrival times
    std::vector<Process> workload = {
        {1, 0, 24, 3, 0, 0, 0, 0, 0, 0},
        {2, 1, 3,  1, 0, 0, 0, 0, 0, 0},
        {3, 2, 3,  2, 0, 0, 0, 0, 0, 0},
        {4, 3, 12, 4, 0, 0, 0, 0, 0, 0},
        {5, 4, 6,  2, 0, 0, 0, 0, 0, 0}
    };

    std::vector<SimulationResult> results;
    results.push_back(simulate_fcfs(workload));
    results.push_back(simulate_sjf(workload));
    results.push_back(simulate_srtf(workload));
    results.push_back(simulate_round_robin(workload, 4));

    print_comparison(results);
    return 0;
}
