#include "shell.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    Shell shell;

    // If commands are passed as arguments (e.g. ./mini_shell -c "echo hi | wc -c")
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        shell.execute_line(argv[2]);
        return 0;
    }

    shell.run();
    return 0;
}
