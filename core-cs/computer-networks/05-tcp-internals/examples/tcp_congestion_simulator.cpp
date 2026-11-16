#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

struct CongestionSimulator {
    static void run() {
        std::cout << "================ TCP CONGESTION CONTROL SIMULATOR ================\n";
        std::cout << "Comparing TCP Reno (AIMD) vs TCP Cubic under identical packet loss.\n";
        std::cout << "Packet drop injected at Round 15.\n\n";

        // Parameters
        constexpr int TOTAL_ROUNDS = 25;
        constexpr int LOSS_ROUND = 15;
        constexpr double INITIAL_SSTHRESH = 32.0;

        // Reno State
        double reno_cwnd = 1.0;
        double reno_ssthresh = INITIAL_SSTHRESH;

        // Cubic State
        double cubic_cwnd = 1.0;
        double cubic_ssthresh = INITIAL_SSTHRESH;
        double w_max = 0.0;
        double time_since_loss = 0.0; // t in seconds
        double K = 0.0;
        constexpr double C = 0.4;
        constexpr double BETA_CUBIC = 0.7; // Cubic multiplicative decrease factor

        std::cout << std::left
                  << std::setw(8)  << "Round"
                  << std::setw(15) << "Reno cwnd"
                  << std::setw(15) << "Reno State"
                  << std::setw(15) << "Cubic cwnd"
                  << std::setw(15) << "Cubic State"
                  << "\n";
        std::cout << std::string(68, '-') << "\n";

        for (int r = 1; r <= TOTAL_ROUNDS; ++r) {
            std::string reno_state;
            std::string cubic_state;

            // 1. Reno Logic
            if (r == LOSS_ROUND) {
                // Packet loss detected by 3 Dup ACKs
                reno_ssthresh = std::max(2.0, reno_cwnd / 2.0);
                reno_cwnd = reno_ssthresh;
                reno_state = "PACKET LOSS!";
            } else if (reno_cwnd < reno_ssthresh) {
                // Slow Start: Exponential
                reno_cwnd = std::min(reno_cwnd * 2.0, reno_ssthresh);
                reno_state = "Slow Start";
            } else {
                // Congestion Avoidance: Linear (+1 per RTT)
                reno_cwnd += 1.0;
                reno_state = "Cong. Avoid";
            }

            // 2. Cubic Logic
            if (r == LOSS_ROUND) {
                w_max = cubic_cwnd;
                cubic_ssthresh = std::max(2.0, w_max * BETA_CUBIC);
                cubic_cwnd = cubic_ssthresh;
                // K = cbrt(w_max * (1 - beta) / C)
                K = std::cbrt(w_max * (1.0 - BETA_CUBIC) / C);
                time_since_loss = 0.0;
                cubic_state = "PACKET LOSS!";
            } else if (r < LOSS_ROUND && cubic_cwnd < cubic_ssthresh) {
                cubic_cwnd = std::min(cubic_cwnd * 2.0, cubic_ssthresh);
                cubic_state = "Slow Start";
            } else {
                if (r > LOSS_ROUND) {
                    time_since_loss += 0.2; // Assume 200ms RTT
                    double offset = time_since_loss - K;
                    cubic_cwnd = C * (offset * offset * offset) + w_max;
                    if (cubic_cwnd < cubic_ssthresh) cubic_cwnd = cubic_ssthresh;
                    cubic_state = "Cubic Concave";
                } else {
                    cubic_cwnd += 1.0;
                    cubic_state = "Cong. Avoid";
                }
            }

            std::cout << std::left
                      << std::setw(8)  << r
                      << std::setw(15) << std::fixed << std::setprecision(1) << reno_cwnd
                      << std::setw(15) << reno_state
                      << std::setw(15) << std::fixed << std::setprecision(1) << cubic_cwnd
                      << std::setw(15) << cubic_state
                      << "\n";
        }

        std::cout << "==================================================================\n";
    }
};

int main() {
    CongestionSimulator::run();
    return 0;
}
