/**
 * @file process_vs_thread_memory.cpp
 * @brief Demonstrates memory isolation in processes (fork) versus memory sharing
 *        and thread-local storage in threads (std::thread).
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic process_vs_thread_memory.cpp -o proc_thread_mem
 *   ./proc_thread_mem
 */

#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

// Global variable in .data
int g_shared_val = 100;

// Thread-local variable: each thread receives an independent copy
thread_local int t_thread_local_val = 500;

void test_process_isolation() {
    std::cout << "--- 1. Testing Process Isolation (fork) ---\n";
    std::cout << "[Parent Before Fork] g_shared_val = " << g_shared_val << "\n";

    pid_t pid = fork();
    if (pid == 0) {
        // In Child Process
        g_shared_val += 50;
        std::cout << "[Child Process] Modified g_shared_val to: " << g_shared_val 
                  << " (Address: " << &g_shared_val << ")\n";
        _exit(0);
    } else {
        // In Parent Process
        waitpid(pid, nullptr, 0); // Wait for child to exit
        std::cout << "[Parent After Child Exit] g_shared_val = " << g_shared_val 
                  << " (Address: " << &g_shared_val << ")\n";
        std::cout << "Observation: Parent value is UNCHANGED despite child modifying it!\n\n";
    }
}

void thread_worker(int id, int add_amount) {
    g_shared_val += add_amount;
    t_thread_local_val += add_amount;

    std::cout << "[Thread " << id << "] Added " << add_amount << ".\n"
              << "   g_shared_val (Shared) = " << g_shared_val 
              << " (Address: " << &g_shared_val << ")\n"
              << "   t_thread_local_val (TLS) = " << t_thread_local_val 
              << " (Address: " << &t_thread_local_val << ")\n";
}

void test_thread_sharing() {
    std::cout << "--- 2. Testing Thread Memory Sharing & TLS ---\n";
    std::cout << "[Main Thread Before Workers] g_shared_val = " << g_shared_val 
              << ", t_thread_local_val = " << t_thread_local_val << "\n";

    std::thread t1(thread_worker, 1, 20);
    t1.join();

    std::thread t2(thread_worker, 2, 30);
    t2.join();

    std::cout << "[Main Thread After Workers] g_shared_val = " << g_shared_val 
              << ", t_thread_local_val = " << t_thread_local_val << "\n";
    std::cout << "Observation: g_shared_val was MUTATED across threads, but t_thread_local_val was private!\n";
}

int main() {
    test_process_isolation();
    test_thread_sharing();
    return 0;
}

/*
EXPECTED OUTPUT:
--- 1. Testing Process Isolation (fork) ---
[Parent Before Fork] g_shared_val = 100
[Child Process] Modified g_shared_val to: 150 (Address: 0x100088000)
[Parent After Child Exit] g_shared_val = 100 (Address: 0x100088000)
Observation: Parent value is UNCHANGED despite child modifying it!

--- 2. Testing Thread Memory Sharing & TLS ---
[Main Thread Before Workers] g_shared_val = 100, t_thread_local_val = 500
[Thread 1] Added 20.
   g_shared_val (Shared) = 120 (Address: 0x100088000)
   t_thread_local_val (TLS) = 520 (Address: 0x16b0a7000)
[Thread 2] Added 30.
   g_shared_val (Shared) = 150 (Address: 0x100088000)
   t_thread_local_val (TLS) = 530 (Address: 0x16b12f000)
[Main Thread After Workers] g_shared_val = 150, t_thread_local_val = 500
Observation: g_shared_val was MUTATED across threads, but t_thread_local_val was private!
*/
