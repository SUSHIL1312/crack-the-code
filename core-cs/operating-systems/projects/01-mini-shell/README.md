# Project 01: Mini Unix Shell

A fully featured POSIX-compliant interactive shell implemented in Modern C++20.

---

## Features
- **Interactive REPL**: Prompt showing current working directory, readline support.
- **Built-in Commands**: `cd`, `pwd`, `exit`, `help`.
- **Command Execution**: PATH resolution via `execvp()`, parent process coordination via `waitpid()`.
- **Pipelining**: Arbitrary multi-command pipelines (`cmd1 | cmd2 | cmd3 | ...`) using UNIX pipes (`pipe()`) and file descriptor duplication (`dup2()`).
- **I/O Redirection**:
  - Input redirection: `< input.txt` (`O_RDONLY`)
  - Output redirection: `> output.txt` (`O_WRONLY | O_CREAT | O_TRUNC`)
  - Append redirection: `>> output.txt` (`O_WRONLY | O_CREAT | O_APPEND`)
- **Background Execution**: Commands ending with `&` run asynchronously without blocking the prompt.
- **Signal Handling**:
  - `SIGINT` (Ctrl+C): Cancels running foreground job without killing the shell prompt.
  - `SIGCHLD`: Asynchronously reaps background zombie child processes.

---

## Build and Run
```bash
make
./mini_shell
```

### Example Test Commands
```bash
mini-shell> pwd
mini-shell> ls -la
mini-shell> echo "Systems programming is great" > test.txt
mini-shell> cat < test.txt | tr 'a-z' 'A-Z'
mini-shell> rm test.txt
mini-shell> sleep 1 &
mini-shell> exit
```
