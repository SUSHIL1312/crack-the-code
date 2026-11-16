/**
 * ============================================================================
 * REFERENCE SOLUTION: Lab 01 Multi-Worker Process Manager
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

    pid_t spawn_worker(void (*task)()) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return -1;
        }

        if (pid == 0) {
            // Child process execution
            task();
            _exit(0); // Use _exit to bypass flushing parent stdio buffers
        }

        // Parent process
        active_workers_.insert(pid);
        return pid;
    }

    int reap_zombies_nonblocking() {
        int reaped_count = 0;
        int status = 0;
        pid_t reaped_pid;

        // WNOHANG ensures waitpid returns 0 immediately if no child has exited
        while ((reaped_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            active_workers_.erase(reaped_pid);
            ++reaped_count;
        }

        return reaped_count;
    }

    void terminate_all_workers() {
        for (pid_t pid : active_workers_) {
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0); // Wait for clean shutdown
        }
        active_workers_.clear();
    }

    size_t active_worker_count() const {
        return active_workers_.size();
    }

private:
    std::unordered_set<pid_t> active_workers_;
};

// ============================================================================
// AUTOMATED TEST SUITE
// ============================================================================

void short_task() {
    _exit(0);
}

void long_task() {
    while (true) {
        pause();
    }
}

void run_test_suite() {
    std::cout << "========================================================\n";
    std::cout << "  RUNNING REFERENCE SOLUTION: Lab 01 Process Manager    \n";
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

    std::cout << "\n🎉 ALL TESTS PASSED! Lab 01 Solution Verified!\n";
}

int main() {
    run_test_suite();
    return 0;
}
