# Solutions: Advanced OS Concepts

---

### Solution 1: Minimal Container Implementation in C++ (Linux Architecture)

Below is a complete, self-contained reference implementation showing how a Linux container runtime executes a command in isolated PID, UTS, and Mount namespaces.

```cpp
#define _GNU_SOURCE
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>

#if defined(__linux__)

int container_entry(void* arg) {
    char** argv = reinterpret_cast<char**>(arg);

    // Step 1: Set isolated container hostname
    if (sethostname("crack-container", 15) != 0) {
        perror("sethostname");
        return 1;
    }

    // Step 2: Ensure parent mount propagation does not leak into container
    if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0) {
        perror("mount MS_PRIVATE");
        return 1;
    }

    // Step 3: Mount a private /proc so ps only shows container processes
    // Note: In a complete container runtime (runc), pivot_root() would run first.
    if (mount("proc", "/proc", "proc", 0, nullptr) != 0) {
        perror("mount /proc");
        // May fail if not root / CAP_SYS_ADMIN
    }

    std::cout << "[Container] Running command as PID: " << getpid() << "\n";

    // Step 4: Execute target command
    execvp(argv[0], argv);
    perror("execvp");
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <command> [args...]\n";
        return 1;
    }

    // Allocate stack for clone
    constexpr size_t STACK_SIZE = 1024 * 1024;
    std::vector<char> stack(STACK_SIZE);
    char* stack_top = stack.data() + STACK_SIZE;

    // Clone into new UTS, PID, and Mount namespaces
    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD;
    pid_t pid = clone(container_entry, stack_top, flags, &argv[1]);

    if (pid < 0) {
        perror("clone");
        std::cerr << "Hint: Creating namespaces requires root privileges or user namespaces enabled.\n";
        return 1;
    }

    std::cout << "[Host] Container spawned with PID: " << pid << "\n";
    int status;
    waitpid(pid, &status, 0);
    std::cout << "[Host] Container exited with status: " << WEXITSTATUS(status) << "\n";

    return 0;
}

#else

int main() {
    std::cout << "This exercise requires a Linux kernel to execute clone() namespace flags.\n"
              << "Run on a Linux machine or inside a Linux Docker VM.\n";
    return 0;
}

#endif
```
