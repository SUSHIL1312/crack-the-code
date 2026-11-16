/**
 * @file hard_vs_soft_links.cpp
 * @brief Demonstrates programmatic creation and behavior of Hard Links vs Symbolic (Soft) Links
 *        using link(), symlink(), unlink(), and stat() / lstat().
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic hard_vs_soft_links.cpp -o link_demo
 *   ./link_demo
 */

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

void print_stat_info(const char* label, const char* path, bool use_lstat = false) {
    struct stat st;
    int res = use_lstat ? lstat(path, &st) : stat(path, &st);
    if (res < 0) {
        std::cout << label << " (" << path << "): [DOES NOT EXIST / BROKEN LINK] (" 
                  << strerror(errno) << ")\n";
        return;
    }

    std::cout << label << " (" << path << "):\n"
              << "  Inode Number: " << st.st_ino << "\n"
              << "  Hard Link Count: " << st.st_nlink << "\n"
              << "  File Size: " << st.st_size << " bytes\n";
}

int main() {
    const char* orig_file = "/tmp/test_orig.txt";
    const char* hard_file = "/tmp/test_hard.txt";
    const char* soft_file = "/tmp/test_soft.txt";

    // Clean up any stale files
    unlink(orig_file);
    unlink(hard_file);
    unlink(soft_file);

    // 1. Create original file with content
    {
        std::ofstream out(orig_file);
        out << "Important File Content: Secret Key 12345\n";
    }
    std::cout << "=== 1. Created Original File ===\n";
    print_stat_info("Original File", orig_file);

    // 2. Create Hard Link via link()
    if (link(orig_file, hard_file) < 0) {
        perror("link failed");
        return 1;
    }
    std::cout << "\n=== 2. Created Hard Link ===\n";
    print_stat_info("Original File", orig_file);
    print_stat_info("Hard Link", hard_file);
    std::cout << "Observation: Both files share the EXACT SAME Inode Number! Hard Link count = 2.\n";

    // 3. Create Symbolic Link via symlink()
    if (symlink(orig_file, soft_file) < 0) {
        perror("symlink failed");
        return 1;
    }
    std::cout << "\n=== 3. Created Symbolic Link ===\n";
    print_stat_info("Symbolic Link (via lstat)", soft_file, true);
    std::cout << "Observation: Symlink has its OWN unique Inode Number!\n";

    // 4. Delete Original File via unlink()
    unlink(orig_file);
    std::cout << "\n=== 4. Unlinked Original File ===\n";
    print_stat_info("Original File", orig_file);
    print_stat_info("Hard Link", hard_file);
    print_stat_info("Symbolic Link (via stat)", soft_file, false);

    // Verify hard link content is completely preserved
    std::ifstream in(hard_file);
    std::string content;
    std::getline(in, content);
    std::cout << "\nRead from Hard Link: \"" << content << "\"\n";
    std::cout << "Conclusion: Hard link preserved the physical data blocks even after original was deleted!\n";

    // Clean up
    unlink(hard_file);
    unlink(soft_file);
    return 0;
}
