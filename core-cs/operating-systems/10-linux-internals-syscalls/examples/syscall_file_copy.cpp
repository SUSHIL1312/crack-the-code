/**
 * @file syscall_file_copy.cpp
 * @brief Production-grade file copy utility implemented purely with raw POSIX system calls:
 *        open(), read(), write(), and close().
 *
 * Demonstrates proper handling of EINTR (interrupted syscall) and partial writes.
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic syscall_file_copy.cpp -o syscall_cp
 *   ./syscall_cp <source_file> <dest_file>
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <vector>

// Choose 8 KB buffer size to align with filesystem block sizes and page cache
constexpr size_t BUFFER_SIZE = 8192;

// Robust helper to write all bytes, handling partial writes and EINTR
bool write_all(int fd, const char* buffer, size_t count) {
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t written = write(fd, buffer + total_written, count - total_written);
        if (written < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, retry write
                continue;
            }
            // Real I/O error
            std::cerr << "write error: " << strerror(errno) << "\n";
            return false;
        }
        total_written += static_cast<size_t>(written);
    }
    return true;
}

bool copy_file(const char* src_path, const char* dst_path) {
    // 1. Open source file for reading
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        std::cerr << "Cannot open source file '" << src_path << "': " << strerror(errno) << "\n";
        return false;
    }

    // 2. Open destination file (Create if doesn't exist, truncate if it does, mode 0644)
    int dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) {
        std::cerr << "Cannot open destination file '" << dst_path << "': " << strerror(errno) << "\n";
        close(src_fd);
        return false;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = 0;
    bool success = true;

    // 3. Read loop
    while (true) {
        bytes_read = read(src_fd, buffer, sizeof(buffer));

        if (bytes_read == 0) {
            // End of File (EOF) reached!
            break;
        }

        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue; // Interrupted by a signal, retry read
            }
            std::cerr << "read error: " << strerror(errno) << "\n";
            success = false;
            break;
        }

        // Write buffer to destination
        if (!write_all(dst_fd, buffer, static_cast<size_t>(bytes_read))) {
            success = false;
            break;
        }
    }

    // 4. Close both file descriptors
    if (close(src_fd) < 0) {
        std::cerr << "Error closing source: " << strerror(errno) << "\n";
    }
    if (close(dst_fd) < 0) {
        std::cerr << "Error closing destination: " << strerror(errno) << "\n";
        success = false;
    }

    return success;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <source_path> <dest_path>\n";
        std::cout << "Running self-test copying source code to /tmp/copy_test.txt...\n";
        if (copy_file(argv[0], "/tmp/copy_test_bin")) {
            std::cout << "Self-test copy succeeded!\n";
            unlink("/tmp/copy_test_bin");
        }
        return 0;
    }

    if (copy_file(argv[1], argv[2])) {
        std::cout << "Successfully copied '" << argv[1] << "' to '" << argv[2] << "'.\n";
        return 0;
    }

    return 1;
}
