#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

#if defined(__linux__)
#include <sched.h>
#include <sys/mount.h>
#include <sys/utsname.h>

// Child process entry inside isolated Linux namespaces
int container_child_main(void* arg) {
    char* hostname = reinterpret_cast<char*>(arg);
    std::cout << "[Container] Started child process inside new PID and UTS namespace.\n";
    std::cout << "[Container] Child observed getpid(): " << getpid() << " (Inside namespace, we are PID 1!)\n";
    std::cout << "[Container] Child observed getppid(): " << getppid() << "\n";

    // Set private hostname in isolated UTS namespace
    if (sethostname(hostname, strlen(hostname)) == 0) {
        struct utsname uts;
        uname(&uts);
        std::cout << "[Container] Child hostname set to: " << uts.nodename << "\n";
    }

    std::cout << "[Container] Execution complete. Exiting namespace...\n";
    return 0;
}

int main() {
    std::cout << "================ LINUX CONTAINER NAMESPACE DEMO ================\n";
    std::cout << "[Host] Host getpid(): " << getpid() << "\n";

    struct utsname host_uts;
    uname(&host_uts);
    std::cout << "[Host] Original host nodename: " << host_uts.nodename << "\n";

    // Allocate stack for child in clone()
    constexpr size_t STACK_SIZE = 1024 * 1024;
    std::vector<char> stack(STACK_SIZE);
    char* stack_top = stack.data() + STACK_SIZE;

    char container_name[] = "isolated-faang-container";

    // clone with new PID and new UTS (hostname) namespaces
    int clone_flags = CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
    pid_t child_pid = clone(container_child_main, stack_top, clone_flags, container_name);

    if (child_pid < 0) {
        std::cerr << "clone() failed: " << strerror(errno) << "\n";
        std::cerr << "(Note: Creating user/PID namespaces requires CAP_SYS_ADMIN or unprivileged userns enabled)\n";
        return 1;
    }

    std::cout << "[Host] Spawned container child with Host PID: " << child_pid << "\n";
    waitpid(child_pid, nullptr, 0);

    // Verify host hostname remains untouched
    uname(&host_uts);
    std::cout << "[Host] Verified host nodename unchanged: " << host_uts.nodename << "\n";
    std::cout << "=================================================================\n";
    return 0;
}

#else

// macOS / non-Linux POSIX demonstration and architectural explanation
int main() {
    std::cout << "================ CONTAINER NAMESPACE ARCHITECTURE ================\n";
    std::cout << "Current OS: macOS / Darwin (Mach microkernel + FreeBSD POSIX layer)\n";
    std::cout << "Note: Linux Namespaces (CLONE_NEWPID, CLONE_NEWUTS, CLONE_NEWNET) are native\n"
              << "to the Linux kernel. macOS containerization (e.g. Docker Desktop on Mac)\n"
              << "runs a lightweight Linux hypervisor VM (via macOS Hypervisor.framework) to host\n"
              << "the Linux kernel which creates these namespaces.\n\n";

    std::cout << "Key Concepts of Container Namespaces:\n";
    std::cout << "  1. CLONE_NEWPID: Child process sees itself as PID 1; cannot observe host PIDs.\n";
    std::cout << "  2. CLONE_NEWUTS: Isolated hostname; changing sethostname() does not affect host.\n";
    std::cout << "  3. CLONE_NEWNET: Isolated network stack (own loopback, routing table, iptables).\n";
    std::cout << "  4. CLONE_NEWNS:  Isolated mount points; mounts do not propagate to host.\n";
    std::cout << "  5. CLONE_NEWUSER: Maps internal UID 0 (root) to unprivileged host UID 1000.\n\n";

    std::cout << "Host Process Details:\n";
    std::cout << "  PID: " << getpid() << ", PPID: " << getppid() << "\n";
    std::cout << "==================================================================\n";
    return 0;
}
#endif
