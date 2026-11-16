/**
 * @file zombie_orphan_demo.cpp
 * @brief Hands-on demonstration of:
 *        1. Creating and observing a Zombie Process (Z / defunct in ps)
 *        2. Creating and observing an Orphan Process (adopted by PID 1)
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic zombie_orphan_demo.cpp -o zombie_orphan
 *   ./zombie_orphan
 */

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

void demonstrate_zombie() {
    std::cout << "=== DEMO 1: Creating a Zombie Process ===\n";
    pid_t pid = fork();

    if (pid == 0) {
        // Child exits immediately
        std::cout << "[Child PID: " << getpid() << "] Exiting now with code 42...\n";
        _exit(42);
    } else {
        // Parent does NOT call waitpid immediately, but sleeps
        std::cout << "[Parent PID: " << getpid() << "] Child created (PID " << pid << ").\n";
        std::cout << "[Parent] Sleeping for 4 seconds without calling waitpid()...\n";
        std::cout << ">>> IN ANOTHER TERMINAL, RUN: ps -o pid,ppid,state,command -p " << pid << "\n";
        std::cout << ">>> You will see STATE 'Z' (Zombie/Defunct)!\n";
        
        sleep(4);

        std::cout << "[Parent] Waking up and calling waitpid() to reap zombie...\n";
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            std::cout << "[Parent] Reaped child exit status: " << WEXITSTATUS(status) << ".\n";
            std::cout << "[Parent] Zombie is now cleared from the kernel process table.\n\n";
        }
    }
}

void demonstrate_orphan() {
    std::cout << "=== DEMO 2: Creating an Orphan Process ===\n";
    pid_t pid = fork();

    if (pid == 0) {
        // Child checks parent PID before and after parent dies
        std::cout << "[Child PID: " << getpid() << "] Initial Parent PPID: " << getppid() << "\n";
        std::cout << "[Child] Sleeping for 2 seconds while parent exits...\n";
        sleep(2);
        std::cout << "[Child PID: " << getpid() << "] Woke up! New Adopted Parent PPID: " 
                  << getppid() << " (PID 1 / systemd / launchd)\n";
        _exit(0);
    } else {
        // Parent exits immediately
        std::cout << "[Parent PID: " << getpid() << "] Exiting immediately, leaving child orphaned!\n";
        _exit(0);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--orphan") {
        demonstrate_orphan();
    } else {
        demonstrate_zombie();
        std::cout << "To test orphan creation, run: ./zombie_orphan --orphan\n";
    }
    return 0;
}
