/**
 * @file fork_exec_wait.cpp
 * @brief Demonstrates robust Unix process creation, program execution, and exit status reaping
 *        using fork(), execvp(), and waitpid().
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic fork_exec_wait.cpp -o fork_exec_wait
 *   ./fork_exec_wait
 */

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    std::cout << "[Parent PID: " << getpid() << "] About to fork child process...\n";

    pid_t pid = fork();

    if (pid < 0) {
        // fork() failed (e.g., system out of PIDs or memory)
        std::cerr << "Fork failed: " << strerror(errno) << "\n";
        return 1;
    } 
    else if (pid == 0) {
        // ======================== CHILD PROCESS ========================
        std::cout << "[Child PID: " << getpid() << ", Parent PPID: " << getppid() 
                  << "] Fork successful. Replacing process image via execvp...\n";

        // Arguments for execvp: must be a null-terminated array of char* pointers
        char* args[] = {
            const_cast<char*>("echo"),
            const_cast<char*>("[Executed via execvp] Hello from child process!"),
            nullptr
        };

        // execvp replaces the entire memory image (.text, .data, heap, stack)
        // If execvp succeeds, it NEVER returns!
        execvp(args[0], args);

        // If we reach this line, execvp failed
        std::cerr << "Exec failed: " << strerror(errno) << "\n";
        _exit(127);
    } 
    else {
        // ======================== PARENT PROCESS ========================
        std::cout << "[Parent PID: " << getpid() << "] Forked child with PID: " << pid 
                  << ". Waiting for child to terminate...\n";

        int status = 0;
        // waitpid suspends parent until child with PID changes state
        pid_t waited_pid = waitpid(pid, &status, 0);

        if (waited_pid == -1) {
            std::cerr << "waitpid error: " << strerror(errno) << "\n";
            return 1;
        }

        std::cout << "[Parent] Successfully reaped child PID " << waited_pid << ".\n";

        // Unpack child exit status using POSIX macros
        if (WIFEXITED(status)) {
            std::cout << "[Parent] Child exited normally with exit code: " 
                      << WEXITSTATUS(status) << "\n";
        } else if (WIFSIGNALED(status)) {
            std::cout << "[Parent] Child killed abnormally by signal: " 
                      << WTERMSIG(status) << "\n";
        }
    }

    std::cout << "[Parent] Lifecycle complete. Exiting cleanly.\n";
    return 0;
}

/*
EXPECTED OUTPUT:
[Parent PID: 12345] About to fork child process...
[Parent PID: 12345] Forked child with PID: 12346. Waiting for child to terminate...
[Child PID: 12346, Parent PPID: 12345] Fork successful. Replacing process image via execvp...
[Executed via execvp] Hello from child process!
[Parent] Successfully reaped child PID 12346.
[Parent] Child exited normally with exit code: 0
[Parent] Lifecycle complete. Exiting cleanly.

INTERNAL LIFECYCLE SUMMARY:
1. fork() copies page tables with Copy-On-Write (COW).
2. Child starts executing at the exact return instruction of fork(), where fork() returns 0.
3. In the parent, fork() returns the positive PID of the new child.
4. execvp() unmaps the child's old memory and loads the 'echo' ELF binary.
5. waitpid() prevents the child from remaining a zombie after exit.
*/
