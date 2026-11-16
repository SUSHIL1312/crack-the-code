#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

// Speed of light in optical fiber (refractive index n ≈ 1.468)
constexpr double SPEED_OF_LIGHT_VACUUM_KM_S = 299792.458;
constexpr double FIBER_REFRACTIVE_INDEX = 1.468;
constexpr double SPEED_OF_LIGHT_FIBER_KM_S = SPEED_OF_LIGHT_VACUUM_KM_S / FIBER_REFRACTIVE_INDEX; // ~204,218 km/s

struct LinkParameters {
    std::string name;
    double distance_km;
    double bandwidth_bps;  // e.g. 1e9 for 1 Gbps
    double packet_size_bytes; // e.g. 1500 for standard MTU
    double queuing_delay_ms;
    double processing_delay_ms;
};

struct DelayResult {
    double trans_delay_ms;
    double prop_delay_ms;
    double queue_delay_ms;
    double proc_delay_ms;
    double total_one_way_ms;
    double rtt_ms;
    double bdp_bytes; // Bandwidth-Delay Product
};

DelayResult calculate_delay(const LinkParameters& link) {
    DelayResult res;

    // Transmission delay: L / R
    double packet_bits = link.packet_size_bytes * 8.0;
    res.trans_delay_ms = (packet_bits / link.bandwidth_bps) * 1000.0;

    // Propagation delay: d / s
    res.prop_delay_ms = (link.distance_km / SPEED_OF_LIGHT_FIBER_KM_S) * 1000.0;

    res.queue_delay_ms = link.queuing_delay_ms;
    res.proc_delay_ms = link.processing_delay_ms;

    res.total_one_way_ms = res.trans_delay_ms + res.prop_delay_ms + res.queue_delay_ms + res.proc_delay_ms;
    res.rtt_ms = res.total_one_way_ms * 2.0;

    // BDP = Bandwidth (bps) * RTT (seconds) / 8 (bytes)
    double rtt_seconds = res.rtt_ms / 1000.0;
    res.bdp_bytes = (link.bandwidth_bps * rtt_seconds) / 8.0;

    return res;
}

void print_report(const LinkParameters& link, const DelayResult& res) {
    std::cout << "================ " << link.name << " ================\n";
    std::cout << "Link Configuration:\n";
    std::cout << "  Physical Distance:     " << link.distance_km << " km\n";
    std::cout << "  Bandwidth:             " << (link.bandwidth_bps / 1e6) << " Mbps (" << (link.bandwidth_bps / 1e9) << " Gbps)\n";
    std::cout << "  Packet Size:           " << link.packet_size_bytes << " bytes (" << (link.packet_size_bytes * 8) << " bits)\n\n";

    std::cout << "Delay Components (One-Way):\n";
    std::cout << "  1. Transmission Delay (L/R): " << std::fixed << std::setprecision(4) << res.trans_delay_ms << " ms\n";
    std::cout << "  2. Propagation Delay (d/s):  " << res.prop_delay_ms << " ms\n";
    std::cout << "  3. Queuing Delay:            " << res.queue_delay_ms << " ms\n";
    std::cout << "  4. Processing Delay:         " << res.proc_delay_ms << " ms\n";
    std::cout << "  --------------------------------------------------\n";
    std::cout << "  Total One-Way Latency:       " << res.total_one_way_ms << " ms\n";
    std::cout << "  Round-Trip Time (RTT):       " << res.rtt_ms << " ms\n\n";

    std::cout << "Bandwidth-Delay Product (BDP):\n";
    std::cout << "  BDP: " << (res.bdp_bytes / 1024.0 / 1024.0) << " MB (" << res.bdp_bytes << " bytes)\n";
    std::cout << "  (Insight: This is the optimal TCP socket buffer size to saturate the link!)\n";
    std::cout << "====================================================================\n\n";
}

int main() {
    // Scenario 1: Datacenter Intra-Rack (10 meters, 100 Gbps link)
    LinkParameters dc{
        "Datacenter East-West Fabric (10m, 100G)",
        0.010, // 10 meters = 0.01 km
        100e9, // 100 Gbps
        1500,  // 1500 bytes MTU
        0.001, // 1 microsecond queuing
        0.0005 // 500 ns processing
    };
    print_report(dc, calculate_delay(dc));

    // Scenario 2: Transatlantic Submarine Cable (New York to London, 5,585 km, 10 Gbps)
    LinkParameters transatlantic{
        "Transatlantic Fiber Cable (NYC to London, 10 Gbps)",
        5585.0, // 5585 km
        10e9,   // 10 Gbps
        1500,   // 1500 bytes
        2.0,    // 2 ms average queuing in transit routers
        0.1     // 100 us processing
    };
    print_report(transatlantic, calculate_delay(transatlantic));

    return 0;
}
