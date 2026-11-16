/**
 * @file bankers_algorithm.cpp
 * @brief Complete implementation of Dijkstra's Banker's Algorithm for Deadlock Avoidance.
 *        Calculates Need Matrix, discovers Safe Sequences, and handles resource requests.
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic bankers_algorithm.cpp -o bankers_demo
 *   ./bankers_demo
 */

#include <iostream>
#include <vector>
#include <numeric>

class BankersAlgorithm {
public:
    BankersAlgorithm(int num_processes, int num_resources,
                     std::vector<int> available,
                     std::vector<std::vector<int>> max_claim,
                     std::vector<std::vector<int>> allocation)
        : N_(num_processes), M_(num_resources),
          available_(std::move(available)),
          max_(std::move(max_claim)),
          allocation_(std::move(allocation)),
          need_(N_, std::vector<int>(M_, 0)) {
        
        // Compute Need Matrix: Need[i][j] = Max[i][j] - Allocation[i][j]
        for (int i = 0; i < N_; ++i) {
            for (int j = 0; j < M_; ++j) {
                need_[i][j] = max_[i][j] - allocation_[i][j];
            }
        }
    }

    bool is_safe_state(std::vector<int>& safe_sequence) const {
        std::vector<int> work = available_;
        std::vector<bool> finish(N_, false);
        safe_sequence.clear();

        int completed = 0;
        while (completed < N_) {
            bool found = false;
            for (int i = 0; i < N_; ++i) {
                if (!finish[i]) {
                    // Check if Need[i] <= Work
                    bool can_allocate = true;
                    for (int j = 0; j < M_; ++j) {
                        if (need_[i][j] > work[j]) {
                            can_allocate = false;
                            break;
                        }
                    }

                    if (can_allocate) {
                        // Simulate process completing and releasing its allocation
                        for (int j = 0; j < M_; ++j) {
                            work[j] += allocation_[i][j];
                        }
                        finish[i] = true;
                        safe_sequence.push_back(i);
                        completed++;
                        found = true;
                    }
                }
            }

            if (!found) {
                // No process can complete with current work vector -> UNSAFE STATE!
                return false;
            }
        }
        return true;
    }

    bool request_resources(int process_id, const std::vector<int>& request) {
        std::cout << "\n>>> Process P" << process_id << " requesting: [";
        for (int r : request) std::cout << r << " ";
        std::cout << "]...\n";

        // Step 1: Check if Request <= Need
        for (int j = 0; j < M_; ++j) {
            if (request[j] > need_[process_id][j]) {
                std::cerr << "Error: Process exceeded its declared maximum claim!\n";
                return false;
            }
        }

        // Step 2: Check if Request <= Available
        for (int j = 0; j < M_; ++j) {
            if (request[j] > available_[j]) {
                std::cout << "Resource not available. Process P" << process_id << " must wait.\n";
                return false;
            }
        }

        // Step 3: Tentatively allocate resources
        for (int j = 0; j < M_; ++j) {
            available_[j] -= request[j];
            allocation_[process_id][j] += request[j];
            need_[process_id][j] -= request[j];
        }

        // Step 4: Check if resulting state is safe
        std::vector<int> safe_seq;
        if (is_safe_state(safe_seq)) {
            std::cout << "Request GRANTED. System remains in a SAFE state.\n";
            std::cout << "Safe execution sequence: ";
            for (size_t i = 0; i < safe_seq.size(); ++i) {
                std::cout << "P" << safe_seq[i] << (i + 1 < safe_seq.size() ? " -> " : "\n");
            }
            return true;
        } else {
            std::cout << "Request REJECTED. Granting would lead to an UNSAFE state (Deadlock Hazard).\n";
            // Roll back allocation
            for (int j = 0; j < M_; ++j) {
                available_[j] += request[j];
                allocation_[process_id][j] -= request[j];
                need_[process_id][j] += request[j];
            }
            return false;
        }
    }

private:
    int N_; // Number of processes
    int M_; // Number of resources
    std::vector<int> available_;
    std::vector<std::vector<int>> max_;
    std::vector<std::vector<int>> allocation_;
    std::vector<std::vector<int>> need_;
};

int main() {
    // 5 Processes (P0, P1, P2, P3, P4) and 3 Resources (A, B, C)
    std::vector<int> available = {3, 3, 2};

    std::vector<std::vector<int>> max_claim = {
        {7, 5, 3}, // P0
        {3, 2, 2}, // P1
        {9, 0, 2}, // P2
        {2, 2, 2}, // P3
        {4, 3, 3}  // P4
    };

    std::vector<std::vector<int>> allocation = {
        {0, 1, 0}, // P0
        {2, 0, 0}, // P1
        {3, 0, 2}, // P2
        {2, 1, 1}, // P3
        {0, 0, 2}  // P4
    };

    BankersAlgorithm banker(5, 3, available, max_claim, allocation);

    std::vector<int> safe_sequence;
    if (banker.is_safe_state(safe_sequence)) {
        std::cout << "System is initially in a SAFE state.\n";
        std::cout << "Safe sequence: ";
        for (size_t i = 0; i < safe_sequence.size(); ++i) {
            std::cout << "P" << safe_sequence[i] << (i + 1 < safe_sequence.size() ? " -> " : "\n");
        }
    } else {
        std::cout << "System is in an UNSAFE state!\n";
    }

    // Test a valid request
    banker.request_resources(1, {1, 0, 2});

    // Test an unsafe request that would lead to deadlock
    banker.request_resources(0, {0, 2, 0});

    return 0;
}
