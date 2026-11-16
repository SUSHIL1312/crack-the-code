#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <iomanip>

struct EthernetFrame {
    std::string src_mac;
    std::string dst_mac;
    std::string payload_type; // "ARP", "IPv4"
    std::string sender_ip;
    std::string target_ip;
    int arp_opcode; // 1 = Request, 2 = Reply
    std::string data;
};

class L2Switch {
public:
    void receive_frame(int ingress_port, const EthernetFrame& frame) {
        // 1. MAC Learning
        cam_table_[frame.src_mac] = ingress_port;
        std::cout << "[Switch] Learned MAC " << frame.src_mac << " on Port " << ingress_port << "\n";

        // 2. Forwarding Decision
        if (frame.dst_mac == "FF:FF:FF:FF:FF:FF") {
            std::cout << "[Switch] Broadcast frame -> Flooding out all other ports.\n";
            flood(ingress_port, frame);
        } else {
            auto it = cam_table_.find(frame.dst_mac);
            if (it != cam_table_.end()) {
                std::cout << "[Switch] Known unicast -> Forwarding directly out Port " << it->second << ".\n";
                deliver_to_port(it->second, frame);
            } else {
                std::cout << "[Switch] Unknown destination MAC " << frame.dst_mac << " -> Flooding (Unknown Unicast).\n";
                flood(ingress_port, frame);
            }
        }
    }

    void connect_port(int port, class Host* host) {
        ports_[port] = host;
    }

    void print_cam_table() const {
        std::cout << "\n--- Current Switch CAM Table ---\n";
        for (const auto& [mac, port] : cam_table_) {
            std::cout << "  MAC: " << mac << " -> Port " << port << "\n";
        }
        std::cout << "--------------------------------\n\n";
    }

private:
    void deliver_to_port(int port, const EthernetFrame& frame);
    void flood(int ingress_port, const EthernetFrame& frame);

    std::unordered_map<std::string, int> cam_table_; // MAC -> Port
    std::unordered_map<int, Host*> ports_;           // Port -> Host
};

class Host {
public:
    Host(std::string name, std::string ip, std::string mac, int port, L2Switch* sw)
        : name_(std::move(name)), ip_(std::move(ip)), mac_(std::move(mac)), port_(port), switch_(sw) {
        switch_->connect_port(port_, this);
    }

    void send_packet_to_ip(const std::string& target_ip, const std::string& data) {
        std::cout << "[" << name_ << "] Sending data to " << target_ip << ": \"" << data << "\"\n";

        auto it = arp_cache_.find(target_ip);
        if (it != arp_cache_.end()) {
            std::cout << "[" << name_ << "] Cache hit! Target MAC: " << it->second << "\n";
            EthernetFrame frame{mac_, it->second, "IPv4", ip_, target_ip, 0, data};
            switch_->receive_frame(port_, frame);
        } else {
            std::cout << "[" << name_ << "] Cache miss for " << target_ip << ". Broadcasting ARP Request...\n";
            EthernetFrame arp_req{
                mac_,
                "FF:FF:FF:FF:FF:FF",
                "ARP",
                ip_,
                target_ip,
                1, // Opcode: Request
                ""
            };
            switch_->receive_frame(port_, arp_req);
        }
    }

    void receive_frame(const EthernetFrame& frame) {
        if (frame.payload_type == "ARP") {
            if (frame.arp_opcode == 1) { // ARP Request
                if (frame.target_ip == ip_) {
                    std::cout << "  [" << name_ << "] Received ARP Request for my IP! Sending ARP Reply...\n";
                    // Learn requester's MAC
                    arp_cache_[frame.sender_ip] = frame.src_mac;

                    EthernetFrame reply{
                        mac_,
                        frame.src_mac,
                        "ARP",
                        ip_,
                        frame.sender_ip,
                        2, // Opcode: Reply
                        ""
                    };
                    switch_->receive_frame(port_, reply);
                }
            } else if (frame.arp_opcode == 2) { // ARP Reply
                if (frame.target_ip == ip_) {
                    // Check for ARP Spoofing
                    auto it = arp_cache_.find(frame.sender_ip);
                    if (it != arp_cache_.end() && it->second != frame.src_mac) {
                        std::cerr << "  [! SECURITY ALERT !] " << name_ << " detected ARP Cache Poisoning for "
                                  << frame.sender_ip << "! Existing MAC: " << it->second
                                  << ", Attacker MAC: " << frame.src_mac << "\n";
                    } else {
                        std::cout << "  [" << name_ << "] Received ARP Reply! Cached " << frame.sender_ip
                                  << " -> " << frame.src_mac << "\n";
                        arp_cache_[frame.sender_ip] = frame.src_mac;
                    }
                }
            }
        } else if (frame.payload_type == "IPv4") {
            if (frame.dst_mac == mac_ && frame.target_ip == ip_) {
                std::cout << "  [" << name_ << "] Received IPv4 Data Packet from " << frame.sender_ip
                          << ": \"" << frame.data << "\"\n";
            }
        }
    }

    const std::string& name() const { return name_; }
    const std::string& ip() const { return ip_; }
    const std::string& mac() const { return mac_; }

private:
    std::string name_;
    std::string ip_;
    std::string mac_;
    int port_;
    L2Switch* switch_;
    std::unordered_map<std::string, std::string> arp_cache_; // IP -> MAC
};

void L2Switch::deliver_to_port(int port, const EthernetFrame& frame) {
    auto it = ports_.find(port);
    if (it != ports_.end()) {
        it->second->receive_frame(frame);
    }
}

void L2Switch::flood(int ingress_port, const EthernetFrame& frame) {
    for (const auto& [p, host] : ports_) {
        if (p != ingress_port) {
            host->receive_frame(frame);
        }
    }
}

int main() {
    std::cout << "================ L2 SWITCH & ARP TABLE SIMULATION ================\n";

    L2Switch net_switch;
    Host host_a("Host_A", "192.168.1.10", "AA:AA:AA:AA:AA:AA", 1, &net_switch);
    Host host_b("Host_B", "192.168.1.20", "BB:BB:BB:BB:BB:BB", 2, &net_switch);
    Host host_c("Host_C", "192.168.1.30", "CC:CC:CC:CC:CC:CC", 3, &net_switch);

    // Scenario 1: Host A sends message to Host B (Triggers ARP Resolution & Switch MAC learning)
    std::cout << "\n--- Step 1: Host A sends to Host B (Empty ARP & CAM tables) ---\n";
    host_a.send_packet_to_ip("192.168.1.20", "Hello Host B!");

    // Host A now has B in ARP cache, send second packet (Unicast without ARP!)
    std::cout << "\n--- Step 2: Host A sends second message to Host B (Warm Cache) ---\n";
    host_a.send_packet_to_ip("192.168.1.20", "Second packet (Unicast directly)");

    net_switch.print_cam_table();

    // Scenario 2: Attacker Host C attempts ARP Cache Poisoning
    std::cout << "\n--- Step 3: Malicious Host C attempts ARP Poisoning on Host A ---\n";
    // Host C claims 192.168.1.20 is at MAC CC:CC:CC:CC:CC:CC
    EthernetFrame rogue_arp{
        "CC:CC:CC:CC:CC:CC",
        "AA:AA:AA:AA:AA:AA",
        "ARP",
        "192.168.1.20", // Spoofing B's IP
        "192.168.1.10",
        2, // Fake ARP Reply
        ""
    };
    net_switch.receive_frame(3, rogue_arp);

    std::cout << "==================================================================\n";
    return 0;
}
