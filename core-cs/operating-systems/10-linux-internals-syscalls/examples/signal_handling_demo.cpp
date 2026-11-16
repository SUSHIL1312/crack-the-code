/**
 * @file signal_handling_demo.cpp
 * @brief Demonstrates robust, safe signal handling in C++ using sigaction()
 *        and atomic flags for graceful shutdown.
 *
 * Compilation:
 *   clang++ -std=c++20 -pthread -Wall -Wextra -Wpedantic signal_handling_demo.cpp -o signal_demo
 *   ./signal_demo
 */

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <atomic>
#include <thread>
#include <chrono>

// Atomic flag accessible across signal handlers and main threads
std::atomic<bool> g_shutdown_requested{false};
std::atomic<int> g_last_signal{0};

// Async-signal-safe handler
void safe_signal_handler(int signum) {
    // RULE: Only perform async-signal-safe operations!
    // No malloc, no new, no std::cout, no printf, no mutex locks!
    g_last_signal.store(signum, std::memory_order_relaxed);
    g_shutdown_requested.store(true, std::memory_order_relaxed);

    // write() is an async-signal-safe system call guaranteed by POSIX
    const char msg[] = "\n[Signal Handler] Intercepted termination signal. Requesting clean exit...\n";
    [[maybe_unused]] auto bytes = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = safe_signal_handler;
    sigemptyset(&sa.sa_mask); // Block no additional signals during handler execution
    sa.sa_flags = SA_RESTART; // Automatically restart interrupted system calls if possible

    // Register handlers for SIGINT (Ctrl+C) and SIGTERM (kill)
    if (sigaction(SIGINT, &sa, nullptr) < 0) {
        perror("sigaction SIGINT");
    }
    if (sigaction(SIGTERM, &sa, nullptr) < 0) {
        perror("sigaction SIGTERM");
    }
}

int main() {
    setup_signal_handlers();

    std::cout << "Process PID: " << getpid() << " running main server loop.\n";
    std::cout << "Send SIGINT (Ctrl+C) or run 'kill -SIGTERM " << getpid() << "' in another terminal...\n";

    // Simulate sending a SIGINT to ourselves after 1 second for self-testing
    std::thread auto_tester([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "[Tester Thread] Simulating SIGINT delivery to process...\n";
        kill(getpid(), SIGINT);
    });

    while (!g_shutdown_requested.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto_tester.join();

    std::cout << "\n[Main Loop] Loop terminated cleanly after signal " 
              << g_last_signal.load() << ".\n";
    std::cout << "[Main Loop] Performing orderly resource deallocation and flush...\n";
    std::cout << "[Main Loop] Server shut down gracefully with exit code 0.\n";

    return 0;
}
