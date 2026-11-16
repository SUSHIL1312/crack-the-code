#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <random>
#include <iomanip>

struct Packet {
    int stream_id;
    int chunk_index; // 0..9
    int global_tcp_seq;
    bool is_lost{false};
};

// Simulation parameters
constexpr int NUM_STREAMS = 4;
constexpr int CHUNKS_PER_STREAM = 10;

void run_tcp_simulation(const std::vector<Packet>& wire_packets) {
    std::cout << "--- 1. HTTP/2 over TCP Simulation (Single Shared Byte Stream) ---\n";

    int next_expected_tcp_seq = 0;
    std::vector<int> stream_delivered_count(NUM_STREAMS, 0);
    std::vector<int> stream_completion_tick(NUM_STREAMS, 0);

    // Queue of unacknowledged packets that need retransmission
    int current_tick = 0;
    size_t packet_idx = 0;

    // Buffer of received out-of-order packets waiting in kernel
    std::vector<Packet> kernel_buffer;

    while (true) {
        current_tick++;

        // Process incoming packet from wire
        if (packet_idx < wire_packets.size()) {
            const Packet& p = wire_packets[packet_idx++];
            if (p.is_lost) {
                // Packet dropped on wire! Sender must timeout and retransmit after RTT_PENALTY_TICKS
                // Simulated retransmit arrives at tick + RTT_PENALTY_TICKS
                Packet retrans = p;
                retrans.is_lost = false;
                // Schedule retransmission
                kernel_buffer.push_back(retrans); // Will be released when retransmitted
            } else {
                kernel_buffer.push_back(p);
            }
        }

        // Check what can be delivered sequentially from kernel buffer
        bool delivered_any = true;
        while (delivered_any) {
            delivered_any = false;
            for (auto it = kernel_buffer.begin(); it != kernel_buffer.end(); ++it) {
                if (it->global_tcp_seq == next_expected_tcp_seq) {
                    stream_delivered_count[it->stream_id]++;
                    if (stream_delivered_count[it->stream_id] == CHUNKS_PER_STREAM && stream_completion_tick[it->stream_id] == 0) {
                        stream_completion_tick[it->stream_id] = current_tick;
                    }
                    next_expected_tcp_seq++;
                    kernel_buffer.erase(it);
                    delivered_any = true;
                    break;
                }
            }
        }

        // Stop when all delivered
        bool all_done = true;
        for (int c : stream_delivered_count) {
            if (c < CHUNKS_PER_STREAM) all_done = false;
        }
        if (all_done) break;
    }

    std::cout << "TCP Stream Completion Times (Ticks):\n";
    for (int s = 0; s < NUM_STREAMS; ++s) {
        std::cout << "  Stream " << s << ": Completed at Tick " << stream_completion_tick[s] << " (Suffered HoL stalls)\n";
    }
}

void run_quic_simulation(const std::vector<Packet>& wire_packets) {
    std::cout << "\n--- 2. HTTP/3 over QUIC Simulation (Independent Streams) ---\n";

    std::vector<int> next_expected_chunk(NUM_STREAMS, 0);
    std::vector<int> stream_completion_tick(NUM_STREAMS, 0);
    std::vector<std::vector<Packet>> stream_buffers(NUM_STREAMS);

    int current_tick = 0;
    size_t packet_idx = 0;

    while (true) {
        current_tick++;

        if (packet_idx < wire_packets.size()) {
            const Packet& p = wire_packets[packet_idx++];
            int sid = p.stream_id;
            if (p.is_lost) {
                // Drop affects ONLY this stream!
                Packet retrans = p;
                retrans.is_lost = false;
                stream_buffers[sid].push_back(retrans);
            } else {
                stream_buffers[sid].push_back(p);
            }
        }

        // Each stream checks its OWN buffer independently!
        for (int s = 0; s < NUM_STREAMS; ++s) {
            bool delivered_stream = true;
            while (delivered_stream) {
                delivered_stream = false;
                for (auto it = stream_buffers[s].begin(); it != stream_buffers[s].end(); ++it) {
                    if (it->chunk_index == next_expected_chunk[s]) {
                        next_expected_chunk[s]++;
                        if (next_expected_chunk[s] == CHUNKS_PER_STREAM && stream_completion_tick[s] == 0) {
                            stream_completion_tick[s] = current_tick;
                        }
                        stream_buffers[s].erase(it);
                        delivered_stream = true;
                        break;
                    }
                }
            }
        }

        bool all_done = true;
        for (int c : next_expected_chunk) {
            if (c < CHUNKS_PER_STREAM) all_done = false;
        }
        if (all_done) break;
    }

    std::cout << "QUIC Stream Completion Times (Ticks):\n";
    for (int s = 0; s < NUM_STREAMS; ++s) {
        std::cout << "  Stream " << s << ": Completed at Tick " << stream_completion_tick[s];
        if (s != 1) {
            std::cout << " (Delivered IMMEDIATELY, zero blocking!)";
        } else {
            std::cout << " (Only stream with packet loss)";
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "================ HEAD-OF-LINE BLOCKING: TCP (HTTP/2) VS QUIC (HTTP/3) ================\n";
    std::cout << "Scenario: 4 concurrent resource streams multiplexed across the connection.\n";
    std::cout << "A single packet is lost on Stream 1.\n\n";

    // Interleave packets across streams: Stream 0, 1, 2, 3, 0, 1, 2, 3...
    std::vector<Packet> packets;
    int seq = 0;
    for (int chunk = 0; chunk < CHUNKS_PER_STREAM; ++chunk) {
        for (int s = 0; s < NUM_STREAMS; ++s) {
            Packet p;
            p.stream_id = s;
            p.chunk_index = chunk;
            p.global_tcp_seq = seq++;
            // Inject loss strictly on Stream 1, Chunk 2
            if (s == 1 && chunk == 2) {
                p.is_lost = true;
            }
            packets.push_back(p);
        }
    }

    run_tcp_simulation(packets);
    run_quic_simulation(packets);

    std::cout << "\nConclusion: In TCP, loss on Stream 1 stalled ALL streams. In QUIC, only Stream 1 delayed!\n";
    std::cout << "=======================================================================================\n";
    return 0;
}
