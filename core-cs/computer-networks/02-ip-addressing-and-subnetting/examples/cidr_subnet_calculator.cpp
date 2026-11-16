#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <arpa/inet.h>
#include <cassert>

class SubnetCalculator {
public:
    static uint32_t ip_to_uint(const std::string& ip_str) {
        struct in_addr addr;
        if (inet_pton(AF_INET, ip_str.c_str(), &addr) <= 0) {
            throw std::invalid_argument("Invalid IPv4 address: " + ip_str);
        }
        return ntohl(addr.s_addr);
    }

    static std::string uint_to_ip(uint32_t ip_uint) {
        struct in_addr addr;
        addr.s_addr = htonl(ip_uint);
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
        return std::string(buf);
    }

    struct SubnetInfo {
        std::string cidr_input;
        std::string ip_address;
        int prefix;
        uint32_t mask_uint;
        std::string subnet_mask;
        std::string wildcard_mask;
        std::string network_address;
        std::string broadcast_address;
        std::string first_usable_host;
        std::string last_usable_host;
        uint64_t total_addresses;
        uint64_t usable_hosts;
    };

    static SubnetInfo calculate(const std::string& cidr_str) {
        size_t slash = cidr_str.find('/');
        if (slash == std::string::npos) {
            throw std::invalid_argument("CIDR must contain '/': " + cidr_str);
        }

        std::string ip_part = cidr_str.substr(0, slash);
        int prefix = std::stoi(cidr_str.substr(slash + 1));
        if (prefix < 0 || prefix > 32) {
            throw std::invalid_argument("Prefix must be between 0 and 32");
        }

        uint32_t ip = ip_to_uint(ip_part);
        uint32_t mask = (prefix == 0) ? 0 : (~0U << (32 - prefix));
        uint32_t wildcard = ~mask;
        uint32_t network = ip & mask;
        uint32_t broadcast = network | wildcard;

        SubnetInfo info;
        info.cidr_input = cidr_str;
        info.ip_address = ip_part;
        info.prefix = prefix;
        info.mask_uint = mask;
        info.subnet_mask = uint_to_ip(mask);
        info.wildcard_mask = uint_to_ip(wildcard);
        info.network_address = uint_to_ip(network);
        info.broadcast_address = uint_to_ip(broadcast);

        uint64_t total = (prefix == 0) ? (1ULL << 32) : (1ULL << (32 - prefix));
        info.total_addresses = total;

        if (prefix == 32) {
            // Single host
            info.first_usable_host = uint_to_ip(network);
            info.last_usable_host = uint_to_ip(network);
            info.usable_hosts = 1;
        } else if (prefix == 31) {
            // RFC 3021 Point-to-Point link: both addresses are usable hosts!
            info.first_usable_host = uint_to_ip(network);
            info.last_usable_host = uint_to_ip(broadcast);
            info.usable_hosts = 2;
        } else {
            info.first_usable_host = uint_to_ip(network + 1);
            info.last_usable_host = uint_to_ip(broadcast - 1);
            info.usable_hosts = total - 2;
        }

        return info;
    }

    static bool contains(const SubnetInfo& sub, const std::string& test_ip) {
        uint32_t target = ip_to_uint(test_ip);
        uint32_t net = ip_to_uint(sub.network_address);
        return (target & sub.mask_uint) == net;
    }

    static void print(const SubnetInfo& info) {
        std::cout << "================ CIDR ANALYSIS: " << info.cidr_input << " ================\n";
        std::cout << "  IP Address:          " << info.ip_address << "\n";
        std::cout << "  Subnet Mask:         " << info.subnet_mask << " (/" << info.prefix << ")\n";
        std::cout << "  Wildcard Mask:       " << info.wildcard_mask << "\n";
        std::cout << "  Network Address:     " << info.network_address << "\n";
        std::cout << "  Broadcast Address:   " << info.broadcast_address << "\n";
        std::cout << "  Usable Host Range:   " << info.first_usable_host << " - " << info.last_usable_host << "\n";
        std::cout << "  Total Addresses:     " << info.total_addresses << "\n";
        std::cout << "  Usable Hosts:        " << info.usable_hosts << "\n";
        std::cout << "=================================================================\n\n";
    }
};

int main() {
    // Test 1: Standard Class C-sized subnet (/26)
    auto s1 = SubnetCalculator::calculate("192.168.1.135/26");
    SubnetCalculator::print(s1);

    assert(s1.network_address == "192.168.1.128");
    assert(s1.broadcast_address == "192.168.1.191");
    assert(s1.usable_hosts == 62);
    assert(SubnetCalculator::contains(s1, "192.168.1.150") == true);
    assert(SubnetCalculator::contains(s1, "192.168.1.200") == false);

    // Test 2: Datacenter point-to-point link (/31 RFC 3021)
    auto s2 = SubnetCalculator::calculate("10.0.0.4/31");
    SubnetCalculator::print(s2);
    assert(s2.usable_hosts == 2);
    assert(s2.first_usable_host == "10.0.0.4");
    assert(s2.last_usable_host == "10.0.0.5");

    // Test 3: Large VPC block (/16)
    auto s3 = SubnetCalculator::calculate("172.16.0.0/16");
    SubnetCalculator::print(s3);
    assert(s3.total_addresses == 65536);
    assert(s3.usable_hosts == 65534);

    std::cout << "All CIDR subnet math verifications: PASSED (100% accurate)!\n";
    return 0;
}
