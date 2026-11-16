#pragma once

#include <string>
#include <vector>
#include <memory>

struct Command {
    std::vector<std::string> args;
    std::string input_file;
    std::string output_file;
    bool append_output{false};
};

struct Pipeline {
    std::vector<Command> commands;
    bool is_background{false};
};

class Shell {
public:
    Shell();
    ~Shell();

    void run();
    bool execute_line(const std::string& line);

private:
    Pipeline parse_pipeline(const std::string& line);
    bool execute_pipeline(const Pipeline& pipeline);
    bool execute_builtin(const Command& cmd);
    bool is_builtin(const std::string& cmd_name) const;

    static void setup_signal_handlers();
};
