/**
 * @file page_replacement_simulator.cpp
 * @brief Comprehensive Page Replacement Algorithm Simulator implementing:
 *        - First-In, First-Out (FIFO)
 *        - Least Recently Used (LRU)
 *        - Optimal Page Replacement (MIN / Belady's Optimal)
 *        - Second-Chance / Clock Algorithm
 *
 * Demonstrates Belady's Anomaly where increasing frames under FIFO increases page faults.
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic page_replacement_simulator.cpp -o pr_sim
 *   ./pr_sim
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <iomanip>

// 1. First-In, First-Out (FIFO)
int simulate_fifo(const std::vector<int>& pages, int num_frames) {
    std::unordered_set<int> memory;
    std::vector<int> fifo_queue;
    int page_faults = 0;

    for (int page : pages) {
        if (memory.find(page) == memory.end()) {
            page_faults++;
            if (static_cast<int>(memory.size()) >= num_frames) {
                // Evict oldest page
                int oldest = fifo_queue.front();
                fifo_queue.erase(fifo_queue.begin());
                memory.erase(oldest);
            }
            memory.insert(page);
            fifo_queue.push_back(page);
        }
    }
    return page_faults;
}

// 2. Least Recently Used (LRU)
int simulate_lru(const std::vector<int>& pages, int num_frames) {
    std::unordered_map<int, std::list<int>::iterator> memory_map;
    std::list<int> lru_list; // Most recently used at front, least at back
    int page_faults = 0;

    for (int page : pages) {
        auto it = memory_map.find(page);
        if (it == memory_map.end()) {
            page_faults++;
            if (static_cast<int>(lru_list.size()) >= num_frames) {
                // Evict least recently used (back of list)
                int lru_page = lru_list.back();
                lru_list.pop_back();
                memory_map.erase(lru_page);
            }
            lru_list.push_front(page);
            memory_map[page] = lru_list.begin();
        } else {
            // Cache hit: Move to front of LRU list
            lru_list.erase(it->second);
            lru_list.push_front(page);
            memory_map[page] = lru_list.begin();
        }
    }
    return page_faults;
}

// 3. Optimal Page Replacement (OPT / MIN)
int simulate_opt(const std::vector<int>& pages, int num_frames) {
    std::unordered_set<int> memory;
    int page_faults = 0;
    int n = static_cast<int>(pages.size());

    for (int i = 0; i < n; ++i) {
        int page = pages[i];
        if (memory.find(page) == memory.end()) {
            page_faults++;
            if (static_cast<int>(memory.size()) >= num_frames) {
                // Find page in memory that will not be used for the longest time in future
                int page_to_evict = -1;
                int furthest_use = -1;

                for (int mem_page : memory) {
                    int next_use = -1;
                    for (int j = i + 1; j < n; ++j) {
                        if (pages[j] == mem_page) {
                            next_use = j;
                            break;
                        }
                    }

                    if (next_use == -1) {
                        // Never used again in future! Best candidate
                        page_to_evict = mem_page;
                        break;
                    }
                    if (next_use > furthest_use) {
                        furthest_use = next_use;
                        page_to_evict = mem_page;
                    }
                }
                memory.erase(page_to_evict);
            }
            memory.insert(page);
        }
    }
    return page_faults;
}

// 4. Clock / Second-Chance Algorithm
int simulate_clock(const std::vector<int>& pages, int num_frames) {
    struct ClockFrame {
        int page = -1;
        bool reference_bit = false;
    };

    std::vector<ClockFrame> frames(num_frames);
    int hand = 0;
    int page_faults = 0;

    for (int page : pages) {
        // Check if page already in frames (Hit)
        bool hit = false;
        for (int i = 0; i < num_frames; ++i) {
            if (frames[i].page == page) {
                frames[i].reference_bit = true;
                hit = true;
                break;
            }
        }

        if (!hit) {
            page_faults++;
            // Advance clock hand looking for reference_bit == false
            while (true) {
                if (frames[hand].page == -1) {
                    // Empty frame
                    frames[hand].page = page;
                    frames[hand].reference_bit = true;
                    hand = (hand + 1) % num_frames;
                    break;
                }
                if (frames[hand].reference_bit) {
                    // Give second chance
                    frames[hand].reference_bit = false;
                    hand = (hand + 1) % num_frames;
                } else {
                    // Evict this page
                    frames[hand].page = page;
                    frames[hand].reference_bit = true;
                    hand = (hand + 1) % num_frames;
                    break;
                }
            }
        }
    }
    return page_faults;
}

int main() {
    // Belady's Anomaly Reference String
    const std::vector<int> belady_string = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    std::cout << "================ PAGE REPLACEMENT SIMULATOR ================\n";
    std::cout << "Reference String: 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5\n\n";

    std::cout << "Algorithm       | 3 Frames | 4 Frames | Belady's Anomaly?\n";
    std::cout << "----------------|----------|----------|------------------\n";

    int fifo_3 = simulate_fifo(belady_string, 3);
    int fifo_4 = simulate_fifo(belady_string, 4);
    std::cout << std::left << std::setw(16) << "FIFO" 
              << "| " << std::setw(9) << fifo_3 
              << "| " << std::setw(9) << fifo_4 
              << "| " << (fifo_4 > fifo_3 ? "YES! (Faults increased)" : "No") << "\n";

    int lru_3 = simulate_lru(belady_string, 3);
    int lru_4 = simulate_lru(belady_string, 4);
    std::cout << std::left << std::setw(16) << "LRU" 
              << "| " << std::setw(9) << lru_3 
              << "| " << std::setw(9) << lru_4 
              << "| " << (lru_4 > lru_3 ? "YES" : "No (Stack Algorithm)") << "\n";

    int opt_3 = simulate_opt(belady_string, 3);
    int opt_4 = simulate_opt(belady_string, 4);
    std::cout << std::left << std::setw(16) << "Optimal (MIN)" 
              << "| " << std::setw(9) << opt_3 
              << "| " << std::setw(9) << opt_4 
              << "| No (Theoretical Minimum)\n";

    int clock_3 = simulate_clock(belady_string, 3);
    int clock_4 = simulate_clock(belady_string, 4);
    std::cout << std::left << std::setw(16) << "Clock (2nd Chance)" 
              << "| " << std::setw(9) << clock_3 
              << "| " << std::setw(9) << clock_4 
              << "| " << (clock_4 > clock_3 ? "YES" : "No") << "\n";

    std::cout << "============================================================\n";

    return 0;
}
