/**
 * tcp_chat.cpp
 *
 * Capstone Project 01: TCP Chat Application with Protocol Framing & Stream Defragmentation
 * Demonstrating length-prefixed protocol framing, network byte ordering, and partial-read handling.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic tcp_chat.cpp -o tcp_chat
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Protocol Message Framing Helper
class MessageFramer {
public:
    // Serializes a string message into wire format: [4-byte big-endian len][body]
    static std::vector<uint8_t> frame(const std::string& message) {
        uint32_t len = htonl(static_cast<uint32_t>(message.size()));
        std::vector<uint8_t> packet(sizeof(uint32_t) + message.size());
        std::memcpy(packet.data(), &len, sizeof(uint32_t));
        std::memcpy(packet.data() + sizeof(uint32_t), message.data(), message.size());
        return packet;
    }

    // Appends newly received bytes and extracts all complete framed messages
    void append_data(const uint8_t* data, size_t size) {
        buffer_.insert(buffer_.end(), data, data + size);
    }

    // Attempts to extract the next complete message. Returns true if message extracted.
    bool extract_next_message(std::string& out_msg) {
        if (buffer_.size() < sizeof(uint32_t)) {
            return false; // Not enough bytes to read header
        }

        uint32_t raw_len;
        std::memcpy(&raw_len, buffer_.data(), sizeof(uint32_t));
        uint32_t payload_len = ntohl(raw_len);

        size_t total_packet_size = sizeof(uint32_t) + payload_len;
        if (buffer_.size() < total_packet_size) {
            return false; // Still waiting for full payload to arrive over TCP
        }

        out_msg.assign(reinterpret_cast<char*>(buffer_.data() + sizeof(uint32_t)), payload_len);
        buffer_.erase(buffer_.begin(), buffer_.begin() + total_packet_size);
        return true;
    }

    void clear() {
        buffer_.clear();
    }

private:
    std::vector<uint8_t> buffer_;
};

// Reliable send helper: loops until all bytes are written to socket
bool send_all(int fd, const uint8_t* data, size_t total_bytes) {
    size_t sent = 0;
    while (sent < total_bytes) {
        ssize_t n = write(fd, data + sent, total_bytes - sent);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return false;
        }
        sent += n;
    }
    return true;
}

// Verification Test Harness: Simulates TCP fragmentation and packet coalescing
void test_tcp_framing_and_fragmentation() {
    std::cout << "=========================================================\n";
    std::cout << "  Project 01: TCP Chat App Framing & Defragmentation Test \n";
    std::cout << "=========================================================\n";

    int sv[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc == 0);
    int client_fd = sv[0];
    int server_fd = sv[1];

    std::vector<std::string> test_messages = {
        "Hello, Alice!",
        "How is the FAANG interview prep going?",
        "Remember that TCP is a stream, not a datagram protocol!",
        "Multi-megabyte payloads require robust framing logic.",
        "Short msg."
    };

    // Serialize all messages into a single contiguous byte buffer (simulates packet coalescing)
    std::vector<uint8_t> coalesced_wire_data;
    for (const auto& msg : test_messages) {
        auto framed = MessageFramer::frame(msg);
        coalesced_wire_data.insert(coalesced_wire_data.end(), framed.begin(), framed.end());
    }

    std::cout << "[Client] Serialized " << test_messages.size() << " chat messages into "
              << coalesced_wire_data.size() << " framed wire bytes.\n";

    // Client thread: Sends data in tiny fragments (e.g. 7 bytes at a time) to simulate MTU splits
    std::thread sender_thread([client_fd, &coalesced_wire_data]() {
        size_t offset = 0;
        constexpr size_t CHUNK_SIZE = 7;
        while (offset < coalesced_wire_data.size()) {
            size_t to_send = std::min(CHUNK_SIZE, coalesced_wire_data.size() - offset);
            bool ok = send_all(client_fd, coalesced_wire_data.data() + offset, to_send);
            assert(ok);
            offset += to_send;
            // Introduce micro-delay to force read fragmentation
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        close(client_fd);
    });

    // Server receiver: Reads arbitrary chunks and defragments messages
    MessageFramer framer;
    std::vector<std::string> received_messages;
    uint8_t read_buf[64];

    while (true) {
        ssize_t bytes_read = read(server_fd, read_buf, sizeof(read_buf));
        if (bytes_read > 0) {
            framer.append_data(read_buf, bytes_read);
            std::string extracted;
            while (framer.extract_next_message(extracted)) {
                received_messages.push_back(extracted);
                std::cout << "  [Server Defragmenter] Extracted (" << extracted.size() 
                          << " bytes): \"" << extracted << "\"\n";
            }
        } else if (bytes_read == 0) {
            // Client closed connection
            break;
        } else {
            if (errno == EINTR) continue;
            perror("read error");
            break;
        }
    }

    sender_thread.join();
    close(server_fd);

    // Verify all messages match
    assert(received_messages.size() == test_messages.size());
    for (size_t i = 0; i < test_messages.size(); ++i) {
        assert(received_messages[i] == test_messages[i]);
    }

    std::cout << "\n[Result] All " << received_messages.size() 
              << " messages successfully defragmented and verified with 100% integrity!\n";
    std::cout << "✅ Capstone Project 01 PASSED!\n";
}

int main() {
    test_tcp_framing_and_fragmentation();
    return 0;
}
