/**
 * ============================================================================
 * LAB 02: Round-Robin CPU Scheduler Simulator
 * ============================================================================
 * 
 * CANDIDATE INSTRUCTIONS:
 * -----------------------
 * CPU Scheduling is the core mechanism by which the operating system multiplexes
 * scarce CPU cores among multiple runnable threads/processes.
 *
 * In this lab, you will implement an preemptive Round-Robin (RR) CPU scheduler.
 *
 * SPECIFICATION & REQUIREMENTS:
 * Process struct:
 *   - `pid`: Process ID
 *   - `arrival_time`: Timestamp when process enters ready queue
 *   - `burst_time`: Total CPU time required
 *   - `remaining_time`: Unexecuted CPU time remaining
 *   - `completion_time`: Timestamp when process completes
 *   - `waiting_time`: Total time spent in ready queue waiting
 *   - `turnaround_time`: `completion_time - arrival_time`
 *
 * Scheduling Rules:
 *   - Ready queue operates on a FIFO basis with a fixed `time_quantum`.
 *   - At each time step, if the ready queue has processes, pick the front process.
 *   - Execute it for `min(time_quantum, remaining_time)`.
 *   - If newly arrived processes appear during or at this time, add them to the queue.
 *   - If the active process has remaining time > 0, re-enqueue it at the back.
 *   - If the active process finishes (`remaining_time == 0`), calculate its:
 *       `completion_time = current_time`
 *       `turnaround_time = completion_time - arrival_time`
 *       `waiting_time = turnaround_time - burst_time`
 *
 * TO RUN TESTS:
 *   make test
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <numeric>

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

    /**
     * Executes the Round Robin scheduling algorithm on the provided processes.
     * Updates each process's completion_time, waiting_time, and turnaround_time.
     * @return ScheduleMetrics containing average waiting and turnaround times.
     */
    ScheduleMetrics schedule(std::vector<Process>& processes) {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. Sort processes by arrival_time if not already sorted.
        // 2. Track current_time.
        // 3. Maintain a ready queue (std::queue<size_t> indices).
        // 4. Advance time, execute front process up to quantum_.
        // 5. Enqueue newly arrived processes, then re-enqueue unfinished active process.
        // 6. When all processes complete, calculate avg waiting & turnaround time.
        (void)processes;
        return {0.0, 0.0};
    }

private:
    int quantum_;
};

// ============================================================================
// AUTOMATED TEST SUITE (DO NOT MODIFY TEST CODE)
// ============================================================================

void run_test_suite() {
    std::cout << "========================================================\n";
    std::cout << "  RUNNING TEST SUITE: Lab 02 Round-Robin Scheduler      \n";
    std::cout << "========================================================\n";

    // Test Scenario: 3 processes arriving at time 0
    // P1: burst = 5
    // P2: burst = 3
    // P3: burst = 1
    // Quantum = 2
    // Timeline:
    // [0-2] P1 (rem 3)
    // [2-4] P2 (rem 1)
    // [4-5] P3 (rem 0) -> finishes at 5. Turnaround = 5, Wait = 5 - 1 = 4.
    // [5-7] P1 (rem 1)
    // [7-8] P2 (rem 0) -> finishes at 8. Turnaround = 8, Wait = 8 - 3 = 5.
    // [8-9] P1 (rem 0) -> finishes at 9. Turnaround = 9, Wait = 9 - 5 = 4.
    // Avg Turnaround = (5 + 8 + 9) / 3 = 22 / 3 = 7.333
    // Avg Wait = (4 + 5 + 4) / 3 = 13 / 3 = 4.333

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
    std::cout << "  ✓ Test 1 Passed: Completion times exact match.\n";

    std::cout << "[TEST 2] Checking Average Turnaround and Waiting Times...\n";
    assert(std::abs(metrics.avg_turnaround_time - (22.0 / 3.0)) < 0.01);
    assert(std::abs(metrics.avg_waiting_time - (13.0 / 3.0)) < 0.01);
    std::cout << "  ✓ Test 2 Passed: Average metrics match theory.\n";

    std::cout << "\n🎉 ALL TESTS PASSED! Lab 02 Successfully Completed!\n";
}

int main() {
    run_test_suite();
    return 0;
}
