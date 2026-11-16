#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <arpa/inet.h>

// Standard Protocol Header Offsets and Wire Structures
struct EthernetHeader {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
};

struct IPv4Header {
    uint8_t ver_ihl;       // 4 bits version, 4 bits IHL
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
};

struct TCPHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t data_offset_res;
    uint8_t flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_ptr;
};

void format_mac(const uint8_t* mac) {
    for (int i = 0; i < 6; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac[i]);
        if (i < 5) std::cout << ":";
    }
    std::cout << std::dec;
}

std::string format_ip(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = ip;
    return inet_ntoa(addr);
}

void dissect_packet(const uint8_t* raw_bytes, size_t len) {
    std::cout << "================ RAW PACKET DISSECTION (" << len << " bytes) ================\n\n";

    if (len < sizeof(EthernetHeader)) {
        std::cerr << "Packet too short for Ethernet header!\n";
        return;
    }

    // 1. Layer 2: Ethernet Header
    const EthernetHeader* eth = reinterpret_cast<const EthernetHeader*>(raw_bytes);
    uint16_t ethertype = ntohs(eth->ethertype);

    std::cout << "[Layer 2: Ethernet Frame]\n";
    std::cout << "  Destination MAC: "; format_mac(eth->dst_mac); std::cout << "\n";
    std::cout << "  Source MAC:      "; format_mac(eth->src_mac); std::cout << "\n";
    std::cout << "  EtherType:       0x" << std::hex << ethertype << std::dec;
    if (ethertype == 0x0800) std::cout << " (IPv4)\n";
    else if (ethertype == 0x86DD) std::cout << " (IPv6)\n";
    else if (ethertype == 0x0806) std::cout << " (ARP)\n";
    else std::cout << " (Other)\n";

    if (ethertype != 0x0800) return;

    // 2. Layer 3: IPv4 Header
    size_t ip_offset = sizeof(EthernetHeader);
    if (len < ip_offset + sizeof(IPv4Header)) {
        std::cerr << "Packet too short for IPv4 header!\n";
        return;
    }

    const IPv4Header* ip = reinterpret_cast<const IPv4Header*>(raw_bytes + ip_offset);
    uint8_t version = (ip->ver_ihl >> 4) & 0x0F;
    uint8_t ihl = (ip->ver_ihl & 0x0F) * 4; // IHL is in 32-bit (4-byte) words
    uint16_t total_len = ntohs(ip->total_length);

    std::cout << "\n[Layer 3: IPv4 Packet]\n";
    std::cout << "  Version:         " << static_cast<int>(version) << "\n";
    std::cout << "  Header Length:   " << static_cast<int>(ihl) << " bytes\n";
    std::cout << "  Total Length:    " << total_len << " bytes\n";
    std::cout << "  TTL:             " << static_cast<int>(ip->ttl) << " hops\n";
    std::cout << "  Protocol:        " << static_cast<int>(ip->protocol);
    if (ip->protocol == 6) std::cout << " (TCP)\n";
    else if (ip->protocol == 17) std::cout << " (UDP)\n";
    else std::cout << " (Other)\n";
    std::cout << "  Source IP:       " << format_ip(ip->src_ip) << "\n";
    std::cout << "  Destination IP:  " << format_ip(ip->dst_ip) << "\n";

    if (ip->protocol != 6) return;

    // 3. Layer 4: TCP Segment
    size_t tcp_offset = ip_offset + ihl;
    if (len < tcp_offset + sizeof(TCPHeader)) {
        std::cerr << "Packet too short for TCP header!\n";
        return;
    }

    const TCPHeader* tcp = reinterpret_cast<const TCPHeader*>(raw_bytes + tcp_offset);
    uint8_t tcp_header_len = ((tcp->data_offset_res >> 4) & 0x0F) * 4;
    uint16_t src_port = ntohs(tcp->src_port);
    uint16_t dst_port = ntohs(tcp->dst_port);
    uint32_t seq = ntohl(tcp->seq_num);
    uint32_t ack = ntohl(tcp->ack_num);
    uint16_t window = ntohs(tcp->window_size);
    uint8_t flags = tcp->flags;

    std::cout << "\n[Layer 4: TCP Segment]\n";
    std::cout << "  Source Port:     " << src_port << "\n";
    std::cout << "  Dest Port:       " << dst_port << (dst_port == 80 ? " (HTTP)" : "") << "\n";
    std::cout << "  Sequence Number: " << seq << "\n";
    std::cout << "  Ack Number:      " << ack << "\n";
    std::cout << "  Header Length:   " << static_cast<int>(tcp_header_len) << " bytes\n";
    std::cout << "  Window Size:     " << window << " bytes\n";
    std::cout << "  Flags:           0x" << std::hex << static_cast<int>(flags) << std::dec << " [";
    if (flags & 0x02) std::cout << " SYN";
    if (flags & 0x10) std::cout << " ACK";
    if (flags & 0x08) std::cout << " PSH";
    if (flags & 0x01) std::cout << " FIN";
    if (flags & 0x04) std::cout << " RST";
    std::cout << " ]\n";

    // 4. Layer 7: Application Payload
    size_t payload_offset = tcp_offset + tcp_header_len;
    if (len > payload_offset) {
        size_t payload_len = len - payload_offset;
        std::cout << "\n[Layer 7: Application Data (" << payload_len << " bytes)]\n";
        std::string payload(reinterpret_cast<const char*>(raw_bytes + payload_offset), payload_len);
        std::cout << "  Payload ASCII:\n\"" << payload << "\"\n";
    }

    std::cout << "=================================================================\n";
}

int main() {
    // Real raw packet capture of an HTTP GET request
    // Ethernet (14 bytes) + IPv4 (20 bytes) + TCP (20 bytes) + Payload ("GET / HTTP/1.1\r\n\r\n")
    const uint8_t raw_packet[] = {
        // Ethernet: Dst (52:54:00:12:34:56), Src (08:00:27:aa:bb:cc), EtherType (0x0800 IPv4)
        0x52, 0x54, 0x00, 0x12, 0x34, 0x56,
        0x08, 0x00, 0x27, 0xaa, 0xbb, 0xcc,
        0x08, 0x00,
        // IPv4: Version 4, IHL 5 (20 bytes), ToS 0, Total Length 58 bytes (0x003A)
        0x45, 0x00, 0x00, 0x3e,
        // ID 0x1c4d, Flags/Offset 0x4000 (Don't Fragment)
        0x1c, 0x4d, 0x40, 0x00,
        // TTL 64, Protocol 6 (TCP), Header Checksum 0x3289
        0x40, 0x06, 0x32, 0x89,
        // Src IP: 192.168.1.50 (0xc0a80132)
        192, 168, 1, 50,
        // Dst IP: 93.184.216.34 (example.com)
        93, 184, 216, 34,
        // TCP: Src Port 54321 (0xd431), Dst Port 80 (0x0050)
        0xd4, 0x31, 0x00, 0x50,
        // Seq Number: 1000 (0x000003e8)
        0x00, 0x00, 0x03, 0xe8,
        // Ack Number: 5000 (0x00001388)
        0x00, 0x00, 0x13, 0x88,
        // Data Offset 5 (20 bytes), Reserved 0, Flags (PSH | ACK = 0x18)
        0x50, 0x18,
        // Window Size: 64240 (0xfaf0)
        0xfa, 0xf0,
        // Checksum 0x6e2a, Urgent Pointer 0
        0x6e, 0x2a, 0x00, 0x00,
        // Payload: "GET / HTTP/1.1\r\n\r\n" (18 bytes)
        'G', 'E', 'T', ' ', '/', ' ', 'H', 'T', 'T', 'P', '/', '1', '.', '1', '\r', '\n', '\r', '\n'
    };

    dissect_packet(raw_packet, sizeof(raw_packet));
    return 0;
}
