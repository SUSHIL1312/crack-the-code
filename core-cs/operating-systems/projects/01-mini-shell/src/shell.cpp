#include "shell.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <csignal>

namespace {
volatile sig_atomic_t g_foreground_pid = 0;

void sigint_handler(int) {
    if (g_foreground_pid > 0) {
        kill(g_foreground_pid, SIGINT);
    } else {
        std::cout << "\nmini-shell> " << std::flush;
    }
}

void sigchld_handler(int) {
    int saved_errno = errno;
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        // Reap all terminated background child processes without blocking
    }
    errno = saved_errno;
}
}

Shell::Shell() {
    setup_signal_handlers();
}

Shell::~Shell() = default;

void Shell::setup_signal_handlers() {
    struct sigaction sa_int{};
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, nullptr);

    struct sigaction sa_chld{};
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, nullptr);
}

bool Shell::is_builtin(const std::string& cmd_name) const {
    return cmd_name == "cd" || cmd_name == "pwd" || cmd_name == "exit" || cmd_name == "help";
}

bool Shell::execute_builtin(const Command& cmd) {
    if (cmd.args.empty()) return true;

    const std::string& name = cmd.args[0];
    if (name == "exit") {
        return false; // Signals shell termination
    } else if (name == "pwd") {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << cwd << "\n";
        } else {
            perror("pwd");
        }
    } else if (name == "cd") {
        const char* target = nullptr;
        if (cmd.args.size() < 2) {
            target = getenv("HOME");
            if (!target) target = "/";
        } else {
            target = cmd.args[1].c_str();
        }
        if (chdir(target) != 0) {
            perror("cd");
        }
    } else if (name == "help") {
        std::cout << "Mini Unix Shell — Built-in commands:\n"
                  << "  cd [dir]    Change working directory\n"
                  << "  pwd         Print current working directory\n"
                  << "  help        Display this help menu\n"
                  << "  exit        Exit the shell\n"
                  << "Supports: pipelines (|), redirection (<, >, >>), and background (&).\n";
    }
    return true;
}

Pipeline Shell::parse_pipeline(const std::string& line) {
    Pipeline pipeline;
    std::string trimmed = line;

    // Check for trailing & (background job)
    size_t last_non_space = trimmed.find_last_not_of(" \t\r\n");
    if (last_non_space != std::string::npos && trimmed[last_non_space] == '&') {
        pipeline.is_background = true;
        trimmed = trimmed.substr(0, last_non_space);
    }

    // Split by pipe '|'
    std::stringstream ss(trimmed);
    std::string segment;
    while (std::getline(ss, segment, '|')) {
        Command cmd;
        std::stringstream token_stream(segment);
        std::string token;

        while (token_stream >> token) {
            if (token == "<") {
                if (token_stream >> token) cmd.input_file = token;
            } else if (token == ">") {
                if (token_stream >> token) {
                    cmd.output_file = token;
                    cmd.append_output = false;
                }
            } else if (token == ">>") {
                if (token_stream >> token) {
                    cmd.output_file = token;
                    cmd.append_output = true;
                }
            } else {
                cmd.args.push_back(token);
            }
        }
        if (!cmd.args.empty()) {
            pipeline.commands.push_back(cmd);
        }
    }
    return pipeline;
}

bool Shell::execute_pipeline(const Pipeline& pipeline) {
    if (pipeline.commands.empty()) return true;

    // Check for single built-in command in parent process
    if (pipeline.commands.size() == 1 && is_builtin(pipeline.commands[0].args[0])) {
        return execute_builtin(pipeline.commands[0]);
    }

    size_t num_cmds = pipeline.commands.size();
    std::vector<int> pipe_prev(2, -1);
    std::vector<int> pipe_next(2, -1);
    std::vector<pid_t> child_pids;

    for (size_t i = 0; i < num_cmds; ++i) {
        if (i < num_cmds - 1) {
            if (pipe(pipe_next.data()) < 0) {
                perror("pipe");
                return true;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return true;
        }

        if (pid == 0) {
            // Child Process
            // If not first command, redirect stdin from pipe_prev[0]
            if (i > 0) {
                dup2(pipe_prev[0], STDIN_FILENO);
                close(pipe_prev[0]);
                close(pipe_prev[1]);
            }

            // If not last command, redirect stdout to pipe_next[1]
            if (i < num_cmds - 1) {
                close(pipe_next[0]);
                dup2(pipe_next[1], STDOUT_FILENO);
                close(pipe_next[1]);
            }

            // Handle file redirections
            const Command& cmd = pipeline.commands[i];
            if (!cmd.input_file.empty()) {
                int in_fd = open(cmd.input_file.c_str(), O_RDONLY);
                if (in_fd < 0) {
                    perror("open input");
                    _exit(1);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (!cmd.output_file.empty()) {
                int flags = O_WRONLY | O_CREAT | (cmd.append_output ? O_APPEND : O_TRUNC);
                int out_fd = open(cmd.output_file.c_str(), flags, 0644);
                if (out_fd < 0) {
                    perror("open output");
                    _exit(1);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            // Built-in execution in subshell if piped
            if (is_builtin(cmd.args[0])) {
                // Execute built-in in subshell
                const_cast<Shell*>(this)->execute_builtin(cmd);
                _exit(0);
            }

            // Prepare argv for execvp
            std::vector<char*> argv;
            for (const auto& arg : cmd.args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            execvp(argv[0], argv.data());
            perror(argv[0]);
            _exit(127);
        }

        // Parent Process: Cleanup pipes
        if (i > 0) {
            close(pipe_prev[0]);
            close(pipe_prev[1]);
        }
        if (i < num_cmds - 1) {
            pipe_prev = pipe_next;
        }
        child_pids.push_back(pid);
    }

    if (pipeline.is_background) {
        std::cout << "[Background job PID: " << child_pids.back() << "]\n";
    } else {
        // Wait for all child processes in the pipeline
        for (pid_t pid : child_pids) {
            g_foreground_pid = pid;
            int status = 0;
            waitpid(pid, &status, 0);
        }
        g_foreground_pid = 0;
    }

    return true;
}

bool Shell::execute_line(const std::string& line) {
    if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
        return true;
    }
    Pipeline pipeline = parse_pipeline(line);
    return execute_pipeline(pipeline);
}

void Shell::run() {
    std::string line;
    while (true) {
        if (isatty(STDIN_FILENO)) {
            std::cout << "mini-shell> " << std::flush;
        }
        if (!std::getline(std::cin, line)) {
            break; // EOF
        }
        if (!execute_line(line)) {
            break; // exit command
        }
    }
}
