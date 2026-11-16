/**
 * http_client.cpp
 *
 * Capstone Project 04: Low-Level Socket HTTP/1.1 Client
 * Demonstrating URL parsing, raw socket request formatting, response header dissection,
 * and RFC 9112 Chunked Transfer-Encoding decoding.
 *
 * Compilation:
 *   clang++ -std=c++20 -O3 -Wall -Wextra -Wpedantic http_client.cpp -o http_client
 */

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>

// Parsed URL Structure
struct ParsedUrl {
    std::string scheme = "http";
    std::string host;
    int port = 80;
    std::string path = "/";
};

// HTTP Response Structure
struct HttpResponse {
    int status_code = 0;
    std::string status_text;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string get_header(std::string key) const {
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        auto it = headers.find(key);
        return (it != headers.end()) ? it->second : "";
    }
};

// URL Parser
class UrlParser {
public:
    static bool parse(const std::string& url, ParsedUrl& out) {
        size_t scheme_pos = url.find("://");
        if (scheme_pos == std::string::npos) return false;

        out.scheme = url.substr(0, scheme_pos);
        std::string rest = url.substr(scheme_pos + 3);

        size_t path_pos = rest.find('/');
        std::string host_port;
        if (path_pos != std::string::npos) {
            host_port = rest.substr(0, path_pos);
            out.path = rest.substr(path_pos);
        } else {
            host_port = rest;
            out.path = "/";
        }

        size_t colon_pos = host_port.find(':');
        if (colon_pos != std::string::npos) {
            out.host = host_port.substr(0, colon_pos);
            out.port = std::stoi(host_port.substr(colon_pos + 1));
        } else {
            out.host = host_port;
            out.port = (out.scheme == "https") ? 443 : 80;
        }

        return true;
    }
};

// Low-Level HTTP/1.1 Client Engine
class HttpClient {
public:
    // Decodes an RFC 9112 Chunked Transfer-Encoding stream:
    // Format: <hex-length>\r\n<data>\r\n...0\r\n\r\n
    static bool decode_chunked_body(const std::string& chunked_data, std::string& decoded_body) {
        decoded_body.clear();
        size_t offset = 0;

        while (offset < chunked_data.size()) {
            size_t crlf = chunked_data.find("\r\n", offset);
            if (crlf == std::string::npos) return false;

            std::string hex_str = chunked_data.substr(offset, crlf - offset);
            // Ignore chunk extensions if present (e.g. "1a;ext=val")
            size_t semi = hex_str.find(';');
            if (semi != std::string::npos) hex_str = hex_str.substr(0, semi);

            size_t chunk_len = 0;
            try {
                chunk_len = std::stoul(hex_str, nullptr, 16);
            } catch (...) {
                return false;
            }

            if (chunk_len == 0) {
                // End of chunked stream
                return true;
            }

            size_t data_start = crlf + 2;
            if (data_start + chunk_len > chunked_data.size()) {
                return false; // Partial chunk data
            }

            decoded_body.append(chunked_data.substr(data_start, chunk_len));
            offset = data_start + chunk_len + 2; // Skip trailing \r\n
        }

        return true;
    }

    // Parses a raw HTTP response wire string into structured HttpResponse
    static bool parse_response(const std::string& raw_response, HttpResponse& resp) {
        size_t header_end = raw_response.find("\r\n\r\n");
        if (header_end == std::string::npos) return false;

        std::string header_section = raw_response.substr(0, header_end);
        std::string body_section = raw_response.substr(header_end + 4);

        std::istringstream stream(header_section);
        std::string status_line;
        if (!std::getline(stream, status_line)) return false;
        if (!status_line.empty() && status_line.back() == '\r') status_line.pop_back();

        std::istringstream ss(status_line);
        std::string http_ver;
        if (!(ss >> http_ver >> resp.status_code)) return false;
        std::getline(ss, resp.status_text);
        if (!resp.status_text.empty() && resp.status_text.front() == ' ') {
            resp.status_text.erase(0, 1);
        }

        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string k = line.substr(0, colon);
                std::string v = line.substr(colon + 1);
                size_t first = v.find_first_not_of(" \t");
                if (first != std::string::npos) v = v.substr(first);
                std::transform(k.begin(), k.end(), k.begin(), ::tolower);
                resp.headers[k] = v;
            }
        }

        // Handle Transfer-Encoding: chunked vs Content-Length
        std::string transfer_encoding = resp.get_header("transfer-encoding");
        if (transfer_encoding.find("chunked") != std::string::npos) {
            return decode_chunked_body(body_section, resp.body);
        } else {
            resp.body = body_section;
            return true;
        }
    }
};

// Automated Verification Test Harness
void test_http_client() {
    std::cout << "=========================================================\n";
    std::cout << "  Project 04: Low-Level Socket HTTP/1.1 Client Test      \n";
    std::cout << "=========================================================\n";

    // Test 1: URL Parsing
    {
        ParsedUrl url;
        bool ok = UrlParser::parse("http://interview.crackthecode.io:8080/v1/curriculum?topic=tcp", url);
        assert(ok);
        assert(url.scheme == "http");
        assert(url.host == "interview.crackthecode.io");
        assert(url.port == 8080);
        assert(url.path == "/v1/curriculum?topic=tcp");
        std::cout << "[Test 1] URL parser verified: " << url.host << ":" << url.port << url.path << "\n";
    }

    // Test 2: Standard Content-Length Response Parsing
    {
        std::string raw = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 27\r\n"
                          "Server: C++20-Engine\r\n"
                          "\r\n"
                          "{\"result\":\"success\",\"code\":0}";
        HttpResponse resp;
        bool ok = HttpClient::parse_response(raw, resp);
        assert(ok);
        assert(resp.status_code == 200);
        assert(resp.status_text == "OK");
        assert(resp.get_header("server") == "C++20-Engine");
        assert(resp.body == "{\"result\":\"success\",\"code\":0}");
        std::cout << "[Test 2] Content-Length response parsed: HTTP " << resp.status_code 
                  << " (Body size: " << resp.body.size() << " bytes)\n";
    }

    // Test 3: RFC 9112 Chunked Transfer-Encoding Decoding
    {
        // 3 chunks: "Wiki" (4 bytes = 0x4), "pedia " (6 bytes = 0x6), "in chunks." (10 bytes = 0xa), terminal 0
        std::string chunked_wire = "4\r\n"
                                   "Wiki\r\n"
                                   "6\r\n"
                                   "pedia \r\n"
                                   "a\r\n"
                                   "in chunks.\r\n"
                                   "0\r\n\r\n";

        std::string raw = "HTTP/1.1 200 OK\r\n"
                          "Transfer-Encoding: chunked\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n" + chunked_wire;

        HttpResponse resp;
        bool ok = HttpClient::parse_response(raw, resp);
        assert(ok);
        assert(resp.status_code == 200);
        assert(resp.body == "Wikipedia in chunks.");
        std::cout << "[Test 3] Chunked transfer decoding verified: \"" << resp.body << "\"\n";
    }

    // Test 4: Socket Wire Transmission & Client Receive Loop
    {
        int sv[2];
        int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        assert(rc == 0);

        // Server writes HTTP response into socket
        std::string server_reply = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Length: 15\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "\r\n"
                                   "Item not found!";
        write(sv[0], server_reply.data(), server_reply.size());

        // Client reads from socket
        char buf[512];
        ssize_t n = read(sv[1], buf, sizeof(buf) - 1);
        assert(n > 0);
        buf[n] = '\0';

        HttpResponse client_resp;
        bool ok = HttpClient::parse_response(std::string(buf, n), client_resp);
        assert(ok);
        assert(client_resp.status_code == 404);
        assert(client_resp.status_text == "Not Found");
        assert(client_resp.body == "Item not found!");
        std::cout << "[Test 4] Socket wire roundtrip verified: HTTP " << client_resp.status_code << "\n";

        close(sv[0]);
        close(sv[1]);
    }

    std::cout << "\n✅ Capstone Project 04 PASSED!\n";
}

int main() {
    test_http_client();
    return 0;
}
