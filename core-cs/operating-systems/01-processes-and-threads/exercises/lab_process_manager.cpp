/**
 * ============================================================================
 * LAB 01: Multi-Worker Process Manager with Non-Blocking Zombie Reaping
 * ============================================================================
 * 
 * CANDIDATE INSTRUCTIONS:
 * -----------------------
 * In production Unix systems (such as Nginx, Gunicorn, or Chrome), a master
 * process spawns and supervises multiple worker processes. If a parent process
 * does not reap terminated child processes, they turn into "Zombies" (<defunct>)
 * consuming entries in the kernel's process table (PID starvation).
 *
 * YOUR TASK:
 * Complete the implementation of `ProcessManager` below by filling in all the
 * `// TODO: CANDIDATE IMPLEMENTATION HERE` blocks.
 *
 * SPECIFICATION & REQUIREMENTS:
 * 1. `spawn_worker(task)`:
 *    - Calls `fork()`.
 *    - In the child process: executes `task()` and exits with code 0.
 *    - In the parent process: records the child's PID into `active_workers_` set
 *      and returns the child's PID.
 *    - If `fork()` fails, return -1.
 *
 * 2. `reap_zombies_nonblocking()`:
 *    - Uses `waitpid()` with the `WNOHANG` flag to check for any terminated child
 *      processes without blocking the parent process.
 *    - CRITICAL: Must reap in a loop because multiple children may have terminated
 *      since the last check (signals can coalesce).
 *    - Removes reaped child PIDs from `active_workers_`.
 *    - Returns the number of child processes successfully reaped.
 *
 * 3. `terminate_all_workers()`:
 *    - Sends `SIGTERM` to every currently active worker process using `kill(pid, SIGTERM)`.
 *    - Waits for them to terminate using `waitpid()`.
 *    - Clears `active_workers_`.
 *
 * 4. `active_worker_count()`:
 *    - Returns the number of currently tracked active workers.
 *
 * TO COMPILE & RUN TESTS:
 *   make test
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cassert>
#include <chrono>
#include <thread>

class ProcessManager {
public:
    ProcessManager() = default;

    ~ProcessManager() {
        terminate_all_workers();
    }

    /**
     * Spawns a child worker process to execute `task`.
     * @param task Function pointer for the worker task to execute.
     * @return PID of the spawned child process, or -1 on failure.
     */
    pid_t spawn_worker(void (*task)()) {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. Call fork()
        // 2. In child: call task(), then call _exit(0)
        // 3. In parent: record child pid in active_workers_ and return pid
        (void)task;
        return -1;
    }

    /**
     * Non-blocking check to reap any terminated child worker processes.
     * @return Number of children reaped in this pass.
     */
    int reap_zombies_nonblocking() {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. Call waitpid(-1, &status, WNOHANG) in a while loop
        // 2. For each reaped child pid (> 0):
        //    - Remove pid from active_workers_
        //    - Increment reaped counter
        // 3. Return reaped counter
        return 0;
    }

    /**
     * Terminates all active child processes using SIGTERM and waits for them.
     */
    void terminate_all_workers() {
        // TODO: CANDIDATE IMPLEMENTATION HERE
        // 1. For each pid in active_workers_:
        //    - kill(pid, SIGTERM)
        //    - waitpid(pid, nullptr, 0)
        // 2. Clear active_workers_
    }

    size_t active_worker_count() const {
        return active_workers_.size();
    }

private:
    std::unordered_set<pid_t> active_workers_;
};

// ============================================================================
// AUTOMATED TEST SUITE (DO NOT MODIFY TEST CODE)
// ============================================================================

void short_task() {
    // Quick task exiting immediately
    _exit(0);
}

void long_task() {
    // Worker that sleeps until killed
    while (true) {
        pause();
    }
}

void run_test_suite() {
    std::cout << "========================================================\n";
    std::cout << "  RUNNING TEST SUITE: Lab 01 Process Manager            \n";
    std::cout << "========================================================\n";

    ProcessManager manager;

    // Test 1: Spawning short-lived workers
    std::cout << "[TEST 1] Spawning 3 short-lived workers...\n";
    pid_t p1 = manager.spawn_worker(short_task);
    pid_t p2 = manager.spawn_worker(short_task);
    pid_t p3 = manager.spawn_worker(short_task);

    assert(p1 > 0 && "Worker 1 PID must be > 0");
    assert(p2 > 0 && "Worker 2 PID must be > 0");
    assert(p3 > 0 && "Worker 3 PID must be > 0");
    assert(manager.active_worker_count() == 3 && "Active worker count should be 3");
    std::cout << "  ✓ Test 1 Passed: Workers successfully spawned.\n";

    // Allow children a few milliseconds to exit
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Test 2: Non-blocking zombie reaping
    std::cout << "[TEST 2] Reaping terminated workers via WNOHANG...\n";
    int reaped = manager.reap_zombies_nonblocking();
    assert(reaped == 3 && "Must reap exactly 3 terminated children");
    assert(manager.active_worker_count() == 0 && "Active worker count should now be 0");
    std::cout << "  ✓ Test 2 Passed: 3 zombies reaped non-blockingly.\n";

    // Test 3: Spawning long-lived workers and terminating them
    std::cout << "[TEST 3] Spawning 2 long-lived workers and terminating all...\n";
    pid_t p4 = manager.spawn_worker(long_task);
    pid_t p5 = manager.spawn_worker(long_task);
    assert(p4 > 0 && p5 > 0);
    assert(manager.active_worker_count() == 2);

    manager.terminate_all_workers();
    assert(manager.active_worker_count() == 0 && "All workers must be terminated and cleaned");
    std::cout << "  ✓ Test 3 Passed: Long-lived workers terminated and cleaned.\n";

    std::cout << "\n🎉 ALL TESTS PASSED! Lab 01 Successfully Completed!\n";
}

int main() {
    run_test_suite();
    return 0;
}
