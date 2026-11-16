#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <iomanip>
#include <arpa/inet.h>
#include <cassert>

// RFC 1071 Internet Checksum Implementation
uint16_t compute_rfc1071_checksum(const void* buffer, size_t num_bytes) {
    const uint16_t* ptr = reinterpret_cast<const uint16_t*>(buffer);
    uint32_t sum = 0;

    // Sum all 16-bit words
    while (num_bytes > 1) {
        sum += *ptr++;
        num_bytes -= 2;
    }

    // If an odd byte remains, pad with zero on the right and add
    if (num_bytes > 0) {
        uint16_t odd_byte = 0;
        *reinterpret_cast<uint8_t*>(&odd_byte) = *reinterpret_cast<const uint8_t*>(ptr);
        sum += odd_byte;
    }

    // Fold 32-bit sum into 16 bits (end-around carry)
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // One's complement (invert bits)
    return static_cast<uint16_t>(~sum);
}

// 12-byte IPv4 Pseudo Header required for TCP and UDP checksums
struct IPv4PseudoHeader {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t reserved{0};
    uint8_t protocol{17}; // 17 = UDP
    uint16_t length{0};   // UDP Header + Payload length
};

struct UDPHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum{0};
};

uint16_t calculate_udp_checksum(const std::string& src_ip_str,
                                const std::string& dst_ip_str,
                                uint16_t src_port,
                                uint16_t dst_port,
                                const std::string& payload) {
    uint16_t udp_total_len = sizeof(UDPHeader) + payload.size();

    IPv4PseudoHeader pseudo{};
    inet_pton(AF_INET, src_ip_str.c_str(), &pseudo.src_ip);
    inet_pton(AF_INET, dst_ip_str.c_str(), &pseudo.dst_ip);
    pseudo.reserved = 0;
    pseudo.protocol = 17; // UDP
    pseudo.length = htons(udp_total_len);

    UDPHeader udp{};
    udp.src_port = htons(src_port);
    udp.dst_port = htons(dst_port);
    udp.length = htons(udp_total_len);
    udp.checksum = 0; // Checksum initialized to 0 during computation

    // Buffer to hold: PseudoHeader + UDPHeader + Payload
    std::vector<uint8_t> buffer(sizeof(IPv4PseudoHeader) + sizeof(UDPHeader) + payload.size());
    size_t offset = 0;

    std::memcpy(buffer.data() + offset, &pseudo, sizeof(pseudo));
    offset += sizeof(pseudo);

    std::memcpy(buffer.data() + offset, &udp, sizeof(udp));
    offset += sizeof(udp);

    std::memcpy(buffer.data() + offset, payload.data(), payload.size());

    uint16_t checksum = compute_rfc1071_checksum(buffer.data(), buffer.size());
    // In UDP, if computed checksum is 0x0000, it is transmitted as 0xFFFF
    return (checksum == 0) ? 0xFFFF : checksum;
}

bool verify_udp_packet(const std::string& src_ip_str,
                       const std::string& dst_ip_str,
                       uint16_t src_port,
                       uint16_t dst_port,
                       uint16_t received_checksum,
                       const std::string& payload) {
    uint16_t udp_total_len = sizeof(UDPHeader) + payload.size();

    IPv4PseudoHeader pseudo{};
    inet_pton(AF_INET, src_ip_str.c_str(), &pseudo.src_ip);
    inet_pton(AF_INET, dst_ip_str.c_str(), &pseudo.dst_ip);
    pseudo.reserved = 0;
    pseudo.protocol = 17;
    pseudo.length = htons(udp_total_len);

    UDPHeader udp{};
    udp.src_port = htons(src_port);
    udp.dst_port = htons(dst_port);
    udp.length = htons(udp_total_len);
    udp.checksum = received_checksum; // Include received checksum

    std::vector<uint8_t> buffer(sizeof(IPv4PseudoHeader) + sizeof(UDPHeader) + payload.size());
    size_t offset = 0;

    std::memcpy(buffer.data() + offset, &pseudo, sizeof(pseudo));
    offset += sizeof(pseudo);

    std::memcpy(buffer.data() + offset, &udp, sizeof(udp));
    offset += sizeof(udp);

    std::memcpy(buffer.data() + offset, payload.data(), payload.size());

    // When including the valid checksum, the ones' complement sum must evaluate to 0x0000
    uint16_t result = compute_rfc1071_checksum(buffer.data(), buffer.size());
    return result == 0;
}

int main() {
    std::cout << "================ RFC 1071 TRANSPORT CHECKSUM VERIFICATION ================\n";

    std::string src_ip = "192.168.1.50";
    std::string dst_ip = "8.8.8.8";
    uint16_t src_port = 54321;
    uint16_t dst_port = 53; // DNS
    std::string payload = "Standard DNS query packet payload data.";

    std::cout << "Packet Configuration:\n";
    std::cout << "  Source:      " << src_ip << ":" << src_port << "\n";
    std::cout << "  Destination: " << dst_ip << ":" << dst_port << "\n";
    std::cout << "  Payload:     \"" << payload << "\" (" << payload.size() << " bytes)\n\n";

    // 1. Compute Checksum at Sender
    uint16_t computed_checksum = calculate_udp_checksum(src_ip, dst_ip, src_port, dst_port, payload);
    std::cout << "Sender computed UDP Checksum: 0x" << std::hex << std::setw(4) << std::setfill('0')
              << computed_checksum << std::dec << "\n";

    // 2. Verify clean packet at Receiver
    bool clean_ok = verify_udp_packet(src_ip, dst_ip, src_port, dst_port, computed_checksum, payload);
    std::cout << "Receiver Verification of Uncorrupted Packet: " << (clean_ok ? "PASSED (Valid!)" : "FAILED") << "\n";
    assert(clean_ok);

    // 3. Simulate bit-flip corruption in transit
    std::cout << "\nSimulating 1-bit memory corruption in payload during transit...\n";
    std::string corrupted_payload = payload;
    corrupted_payload[0] ^= 0x01; // Flip 1 bit in first character

    bool corrupted_ok = verify_udp_packet(src_ip, dst_ip, src_port, dst_port, computed_checksum, corrupted_payload);
    std::cout << "Receiver Verification of Corrupted Packet: "
              << (corrupted_ok ? "FAILED (Undetected)" : "PASSED (Corruption successfully caught and discarded!)") << "\n";
    assert(!corrupted_ok);

    std::cout << "\n===========================================================================\n";
    return 0;
}
