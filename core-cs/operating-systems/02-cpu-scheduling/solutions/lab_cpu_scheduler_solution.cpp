/**
 * ============================================================================
 * REFERENCE SOLUTION: Lab 02 Round-Robin CPU Scheduler Simulator
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <cmath>

struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time = 0;
    int waiting_time = 0;
    int turnaround_time = 0;

    Process(int id, int arr, int burst)
        : pid(id), arrival_time(arr), burst_time(burst), remaining_time(burst) {}
};

struct ScheduleMetrics {
    double avg_waiting_time = 0.0;
    double avg_turnaround_time = 0.0;
};

class RoundRobinScheduler {
public:
    explicit RoundRobinScheduler(int quantum) : quantum_(quantum) {}

    ScheduleMetrics schedule(std::vector<Process>& processes) {
        if (processes.empty()) return {0.0, 0.0};

        size_t n = processes.size();
        std::queue<size_t> ready_queue;
        std::vector<bool> has_arrived(n, false);
        int current_time = 0;
        size_t completed = 0;

        auto check_new_arrivals = [&](int time_limit) {
            for (size_t i = 0; i < n; ++i) {
                if (!has_arrived[i] && processes[i].arrival_time <= time_limit) {
                    ready_queue.push(i);
                    has_arrived[i] = true;
                }
            }
        };

        // Advance to first arrival
        int min_arrival = processes[0].arrival_time;
        for (const auto& p : processes) min_arrival = std::min(min_arrival, p.arrival_time);
        current_time = min_arrival;
        check_new_arrivals(current_time);

        while (completed < n) {
            if (ready_queue.empty()) {
                int next_arr = 1e9;
                for (size_t i = 0; i < n; ++i) {
                    if (!has_arrived[i]) {
                        next_arr = std::min(next_arr, processes[i].arrival_time);
                    }
                }
                current_time = next_arr;
                check_new_arrivals(current_time);
                continue;
            }

            size_t idx = ready_queue.front();
            ready_queue.pop();

            int slice = std::min(quantum_, processes[idx].remaining_time);
            current_time += slice;
            processes[idx].remaining_time -= slice;

            // Check for any processes arriving while this process was executing
            check_new_arrivals(current_time);

            if (processes[idx].remaining_time > 0) {
                // Active process rejoins back of ready queue
                ready_queue.push(idx);
            } else {
                // Process completes
                processes[idx].completion_time = current_time;
                processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
                processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
                completed++;
            }
        }

        double total_wait = 0.0;
        double total_turnaround = 0.0;
        for (const auto& p : processes) {
            total_wait += p.waiting_time;
            total_turnaround += p.turnaround_time;
        }

        return { total_wait / n, total_turnaround / n };
    }

private:
    int quantum_;
};

// ============================================================================
// AUTOMATED TEST SUITE
// ============================================================================

void run_test_suite() {
    std::cout << "========================================================\n";
    std::cout << "  RUNNING REFERENCE SOLUTION: Lab 02 RR Scheduler       \n";
    std::cout << "========================================================\n";

    std::vector<Process> procs = {
        Process(1, 0, 5),
        Process(2, 0, 3),
        Process(3, 0, 1)
    };

    RoundRobinScheduler rr(2);
    ScheduleMetrics metrics = rr.schedule(procs);

    std::cout << "[TEST 1] Checking Process Completion Times...\n";
    assert(procs[0].completion_time == 9 && "P1 completion time must be 9");
    assert(procs[1].completion_time == 8 && "P2 completion time must be 8");
    assert(procs[2].completion_time == 5 && "P3 completion time must be 5");
    std::cout << "  ✓ Test 1 Passed: Completion times exact match (9, 8, 5).\n";

    std::cout << "[TEST 2] Checking Average Turnaround and Waiting Times...\n";
    assert(std::abs(metrics.avg_turnaround_time - (22.0 / 3.0)) < 0.01);
    assert(std::abs(metrics.avg_waiting_time - (13.0 / 3.0)) < 0.01);
    std::cout << "  ✓ Test 2 Passed: Average metrics match theory.\n";

    std::cout << "\n🎉 ALL TESTS PASSED! Lab 02 Solution Verified!\n";
}

int main() {
    run_test_suite();
    return 0;
}
