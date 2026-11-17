/**
 * @file thread_vs_process_sync.cpp
 * @brief Demonstrates the architectural difference between Thread Synchronization
 *        and Process Synchronization on POSIX/Linux systems.
 *
 * Concepts Demonstrated:
 *   1. Thread Sync: Direct shared memory pointer + std::mutex (User-space fast-path).
 *   2. Process Sync: MMU-isolated address spaces + POSIX shared memory (mmap MAP_SHARED)
 *      + Pthread process-shared mutex (PTHREAD_PROCESS_SHARED).
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic thread_vs_process_sync.cpp -o thread_vs_process
 *   ./thread_vs_process
 */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <cassert>

// Shared structure placed inside mmap physical RAM for process synchronization
struct ProcessSharedData {
    pthread_mutex_t mutex;
    int counter;
};

// ============================================================================
// PART 1: THREAD SYNCHRONIZATION (Shared Address Space)
// ============================================================================
void demonstrate_thread_synchronization() {
    std::cout << "--- [Part 1] Thread Synchronization (Shared Virtual Memory) ---\n";
    constexpr int NUM_THREADS = 4;
    constexpr int INCREMENTS_PER_THREAD = 10000;
    constexpr int EXPECTED = NUM_THREADS * INCREMENTS_PER_THREAD;

    int shared_counter = 0;
    std::mutex mtx;
    std::vector<std::jthread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            for (int k = 0; k < INCREMENTS_PER_THREAD; ++k) {
                std::lock_guard<std::mutex> lock(mtx);
                ++shared_counter;
            }
        });
    }

    threads.clear(); // Joins all jthreads
    std::cout << "  Thread Counter Result: " << shared_counter << " / " << EXPECTED << "\n";
    assert(shared_counter == EXPECTED && "Thread counter must match expected increments!");
    std::cout << "  [PASS] All threads synchronized using std::mutex in shared address space.\n\n";
}

// ============================================================================
// PART 2: PROCESS SYNCHRONIZATION (MMU Isolated Memory + PTHREAD_PROCESS_SHARED)
// ============================================================================
void demonstrate_process_synchronization() {
    std::cout << "--- [Part 2] Process Synchronization (PTHREAD_PROCESS_SHARED + mmap) ---\n";
    constexpr int NUM_CHILDREN = 4;
    constexpr int INCREMENTS_PER_CHILD = 10000;
    constexpr int EXPECTED = NUM_CHILDREN * INCREMENTS_PER_CHILD;

    // Allocate shared physical memory visible across fork()
    ProcessSharedData* shared = static_cast<ProcessSharedData*>(
        mmap(nullptr, sizeof(ProcessSharedData),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0)
    );

    assert(shared != MAP_FAILED && "mmap failed to allocate shared memory!");
    shared->counter = 0;

    // Configure pthread mutex for inter-process sharing
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int res = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    assert(res == 0 && "Failed to set PTHREAD_PROCESS_SHARED!");

    pthread_mutex_init(&shared->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    std::vector<pid_t> child_pids;

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Inside Child Process (Own MMU address space!)
            for (int k = 0; k < INCREMENTS_PER_CHILD; ++k) {
                pthread_mutex_lock(&shared->mutex);
                shared->counter++;
                pthread_mutex_unlock(&shared->mutex);
            }
            _exit(0); // Exit child immediately
        } else {
            child_pids.push_back(pid);
        }
    }

    // Parent waits for all child processes to finish
    for (pid_t pid : child_pids) {
        int status = 0;
        waitpid(pid, &status, 0);
    }

    std::cout << "  Process Counter Result: " << shared->counter << " / " << EXPECTED << "\n";
    assert(shared->counter == EXPECTED && "Process counter must match expected increments!");
    std::cout << "  [PASS] All processes safely synchronized across isolated address spaces.\n\n";

    // Clean up
    pthread_mutex_destroy(&shared->mutex);
    munmap(shared, sizeof(ProcessSharedData));
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Thread vs. Process Synchronization Deep Dive          \n";
    std::cout << "========================================================\n\n";

    demonstrate_thread_synchronization();
    demonstrate_process_synchronization();

    std::cout << "All Thread & Process synchronization tests PASSED with 100% accuracy!\n";
    return 0;
}
