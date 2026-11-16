/**
 * file_transfer.cpp
 *
 * Capstone Project 06: Multithreaded Chunked File Transfer Protocol
 * Demonstrating chunk framing, CRC32 integrity verification, pipelined transmission,
 * and reliable reassembly.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic file_transfer.cpp -o file_transfer
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <iomanip>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Standard CRC32 Implementation (IEEE 802.3 polynomial: 0xEDB88320)
class CRC32 {
public:
    static uint32_t calculate(const uint8_t* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; ++i) {
            uint8_t byte = data[i];
            crc ^= byte;
            for (int j = 0; j < 8; ++j) {
                uint32_t mask = -(crc & 1);
                crc = (crc >> 1) ^ (0xEDB88320 & mask);
            }
        }
        return ~crc;
    }
};

// Packed Binary Chunk Header (20 bytes)
#pragma pack(push, 1)
struct ChunkHeader {
    uint32_t file_id;
    uint32_t chunk_index;
    uint32_t total_chunks;
    uint32_t payload_bytes;
    uint32_t crc32_checksum;
};
#pragma pack(pop)

// Reliable socket send loop
static bool write_exact(int fd, const void* buf, size_t len) {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buf);
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, ptr + total, len - total);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return false;
        }
        total += n;
    }
    return true;
}

// Reliable socket read loop
static bool read_exact(int fd, void* buf, size_t len) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, ptr + total, len - total);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return false;
        }
        total += n;
    }
    return true;
}

static constexpr size_t TOTAL_FILE_SIZE = 16 * 1024 * 1024; // 16 MB
static constexpr size_t CHUNK_SIZE = 64 * 1024;             // 64 KB per chunk
static constexpr uint32_t NUM_CHUNKS = (TOTAL_FILE_SIZE + CHUNK_SIZE - 1) / CHUNK_SIZE;

// Verification Test Harness
void test_chunked_file_transfer() {
    std::cout << "=========================================================\n";
    std::cout << "  Project 06: Chunked File Transfer Protocol Test        \n";
    std::cout << "=========================================================\n";

    std::cout << "[Init] Synthesizing " << (TOTAL_FILE_SIZE / (1024 * 1024)) << " MB file (" 
              << NUM_CHUNKS << " chunks of " << (CHUNK_SIZE / 1024) << " KB each)...\n";

    std::vector<uint8_t> source_file(TOTAL_FILE_SIZE);
    for (size_t i = 0; i < TOTAL_FILE_SIZE; ++i) {
        source_file[i] = static_cast<uint8_t>((i * 137 + 43) & 0xFF);
    }

    uint32_t expected_total_crc = CRC32::calculate(source_file.data(), source_file.size());
    std::cout << "[Source] Source File CRC32: 0x" << std::hex << std::uppercase << expected_total_crc 
              << std::dec << "\n";

    int sv[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc == 0);
    int sender_fd = sv[0];
    int receiver_fd = sv[1];

    auto start_time = std::chrono::steady_clock::now();

    // Sender Thread: Chunks the file, calculates chunk CRC32, frames header, and transmits
    std::thread sender_thread([sender_fd, &source_file]() {
        for (uint32_t idx = 0; idx < NUM_CHUNKS; ++idx) {
            size_t offset = idx * CHUNK_SIZE;
            size_t payload_len = std::min(CHUNK_SIZE, source_file.size() - offset);
            const uint8_t* payload_ptr = source_file.data() + offset;

            ChunkHeader hdr;
            hdr.file_id = htonl(101);
            hdr.chunk_index = htonl(idx);
            hdr.total_chunks = htonl(NUM_CHUNKS);
            hdr.payload_bytes = htonl(static_cast<uint32_t>(payload_len));
            hdr.crc32_checksum = htonl(CRC32::calculate(payload_ptr, payload_len));

            // Transmit header + payload
            bool ok = write_exact(sender_fd, &hdr, sizeof(hdr));
            assert(ok);
            ok = write_exact(sender_fd, payload_ptr, payload_len);
            assert(ok);
        }
        close(sender_fd);
    });

    // Receiver Thread: Receives headers, reads payloads, verifies chunk CRC32, and reconstructs file
    std::vector<uint8_t> destination_file(TOTAL_FILE_SIZE, 0);
    uint32_t chunks_received = 0;
    size_t bytes_received = 0;

    while (chunks_received < NUM_CHUNKS) {
        ChunkHeader wire_hdr;
        if (!read_exact(receiver_fd, &wire_hdr, sizeof(wire_hdr))) {
            break;
        }

        uint32_t chunk_idx = ntohl(wire_hdr.chunk_index);
        uint32_t payload_len = ntohl(wire_hdr.payload_bytes);
        uint32_t expected_chunk_crc = ntohl(wire_hdr.crc32_checksum);

        size_t dest_offset = chunk_idx * CHUNK_SIZE;
        assert(dest_offset + payload_len <= TOTAL_FILE_SIZE);

        // Read chunk payload directly into destination file buffer
        bool ok = read_exact(receiver_fd, destination_file.data() + dest_offset, payload_len);
        assert(ok);

        // Verify chunk-level CRC32
        uint32_t actual_chunk_crc = CRC32::calculate(destination_file.data() + dest_offset, payload_len);
        assert(actual_chunk_crc == expected_chunk_crc);

        chunks_received++;
        bytes_received += payload_len;

        // Print progress every 64 chunks (4 MB)
        if (chunks_received % 64 == 0 || chunks_received == NUM_CHUNKS) {
            double pct = (static_cast<double>(chunks_received) / NUM_CHUNKS) * 100.0;
            std::cout << "  [Progress] Received " << chunks_received << "/" << NUM_CHUNKS 
                      << " chunks (" << std::fixed << std::setprecision(1) << pct 
                      << "% - " << (bytes_received / (1024 * 1024)) << " MB)\n";
        }
    }

    sender_thread.join();
    close(receiver_fd);

    auto end_time = std::chrono::steady_clock::now();
    double duration_s = std::chrono::duration<double>(end_time - start_time).count();
    double mb_per_sec = (static_cast<double>(TOTAL_FILE_SIZE) / (1024.0 * 1024.0)) / duration_s;

    std::cout << "[Receiver] Transfer Completed: " << (TOTAL_FILE_SIZE / (1024 * 1024)) 
              << " MB in " << (duration_s * 1000.0) << " ms (" 
              << std::fixed << std::setprecision(2) << mb_per_sec << " MB/s)\n";

    // End-to-End File Hash Verification
    uint32_t actual_total_crc = CRC32::calculate(destination_file.data(), destination_file.size());
    std::cout << "[Verify] Reassembled File CRC32: 0x" << std::hex << std::uppercase << actual_total_crc 
              << std::dec << "\n";
    assert(actual_total_crc == expected_total_crc);
    assert(source_file == destination_file);

    std::cout << "\n[Integrity] Reassembled file bit-for-bit matches original (100% verified)!\n";
    std::cout << "✅ Capstone Project 06 PASSED!\n";
}

int main() {
    test_chunked_file_transfer();
    return 0;
}
