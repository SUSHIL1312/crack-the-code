#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <deque>

struct NetworkProfile {
    std::string name;
    double bandwidth_gbps;
    double rtt_ms;
};

void print_bdp_analysis() {
    std::cout << "================ BANDWIDTH-DELAY PRODUCT (BDP) ANALYSIS ================\n";
    std::vector<NetworkProfile> profiles = {
        {"Datacenter Intra-Rack (100 Gbps, 0.05 ms)", 100.0, 0.05},
        {"Cross-Country Metro (40 Gbps, 20 ms)",       40.0,  20.0},
        {"Transatlantic WAN (10 Gbps, 75 ms)",         10.0,  75.0},
        {"Transpacific Submarine (100 Gbps, 130 ms)",  100.0, 130.0},
        {"Starlink LEO Satellite (200 Mbps, 35 ms)",    0.2,  35.0}
    };

    std::cout << std::left
              << std::setw(42) << "Network Scenario"
              << std::setw(15) << "BDP (Bytes)"
              << std::setw(15) << "BDP (MB)"
              << "Max Rate w/ 64KB Window\n";
    std::cout << std::string(88, '-') << "\n";

    for (const auto& p : profiles) {
        double rtt_sec = p.rtt_ms / 1000.0;
        double bandwidth_bps = p.bandwidth_gbps * 1e9;
        double bdp_bytes = (bandwidth_bps * rtt_sec) / 8.0;
        double bdp_mb = bdp_bytes / (1024.0 * 1024.0);

        // Max throughput with legacy 64 KB (65535 bytes) window
        double max_throughput_bps = (65535.0 * 8.0) / rtt_sec;
        double max_mbps = max_throughput_bps / 1e6;

        std::cout << std::left
                  << std::setw(42) << p.name
                  << std::setw(15) << static_cast<uint64_t>(bdp_bytes)
                  << std::setw(15) << std::fixed << std::setprecision(2) << bdp_mb
                  << std::fixed << std::setprecision(1) << max_mbps << " Mbps\n";
    }
    std::cout << "========================================================================\n\n";
}

struct QueuedPacket {
    double enter_time_ms;
    size_t size_bytes;
};

void simulate_bufferbloat_vs_codel() {
    std::cout << "================ BUFFERBLOAT SIMULATION: DROP-TAIL VS CODEL ================\n";
    std::cout << "Scenario: 10 Mbps bottleneck link (1.25 MB/s). 20 packets/ms burst arrival.\n\n";

    constexpr double DRAIN_RATE_BYTES_PER_MS = (10.0 * 1e6 / 8.0) / 1000.0; // 1250 bytes/ms
    constexpr double SIMULATION_TICKS_MS = 20.0;

    // Simulation 1: Dumb Drop-Tail Queue (Max 100 KB)
    std::deque<QueuedPacket> drop_tail_queue;
    constexpr size_t DROP_TAIL_MAX_BYTES = 100 * 1024;
    size_t drop_tail_current_bytes = 0;
    double max_drop_tail_latency = 0.0;

    // Simulation 2: CoDel AQM Queue (Target = 5 ms sojourn)
    std::deque<QueuedPacket> codel_queue;
    size_t codel_drops = 0;
    double max_codel_latency = 0.0;

    for (double t = 0.0; t < SIMULATION_TICKS_MS; t += 1.0) {
        // Influx: 2 packets of 1000 bytes every ms
        for (int i = 0; i < 2; ++i) {
            // Drop-Tail
            if (drop_tail_current_bytes + 1000 <= DROP_TAIL_MAX_BYTES) {
                drop_tail_queue.push_back({t, 1000});
                drop_tail_current_bytes += 1000;
            }

            // CoDel Queue
            codel_queue.push_back({t, 1000});
        }

        // Drain Drop-Tail Queue at DRAIN_RATE
        double drain_allowance = DRAIN_RATE_BYTES_PER_MS;
        while (!drop_tail_queue.empty() && drain_allowance >= drop_tail_queue.front().size_bytes) {
            auto pkt = drop_tail_queue.front();
            drop_tail_queue.pop_front();
            drop_tail_current_bytes -= pkt.size_bytes;
            drain_allowance -= pkt.size_bytes;

            double sojourn = t - pkt.enter_time_ms;
            if (sojourn > max_drop_tail_latency) max_drop_tail_latency = sojourn;
        }

        // Drain CoDel Queue
        drain_allowance = DRAIN_RATE_BYTES_PER_MS;
        while (!codel_queue.empty() && drain_allowance >= codel_queue.front().size_bytes) {
            auto pkt = codel_queue.front();
            double sojourn = t - pkt.enter_time_ms;

            // If sojourn exceeds 5ms target, CoDel drops early to signal TCP!
            if (sojourn > 5.0 && codel_queue.size() > 2) {
                codel_queue.pop_front();
                codel_drops++;
                continue;
            }

            codel_queue.pop_front();
            drain_allowance -= pkt.size_bytes;
            if (sojourn > max_codel_latency) max_codel_latency = sojourn;
        }
    }

    std::cout << "Simulation Results:\n";
    std::cout << "  Drop-Tail Buffer (No AQM):\n";
    std::cout << "    Max Queuing Latency (Bufferbloat): " << max_drop_tail_latency << " ms\n";
    std::cout << "    Remaining Buffer Congestion:       " << drop_tail_current_bytes << " bytes\n\n";

    std::cout << "  CoDel Controlled Delay Queue:\n";
    std::cout << "    Max Queuing Latency:               " << max_codel_latency << " ms (Kept near 5ms!)\n";
    std::cout << "    Early Congestion Warning Drops:    " << codel_drops << " packets\n";
    std::cout << "============================================================================\n";
}

int main() {
    print_bdp_analysis();
    simulate_bufferbloat_vs_codel();
    return 0;
}
