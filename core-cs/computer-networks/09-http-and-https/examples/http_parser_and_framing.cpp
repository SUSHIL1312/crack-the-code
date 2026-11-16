#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cassert>

// Case-insensitive string equality for header keys
struct HeaderHash {
    size_t operator()(const std::string& str) const {
        size_t hash = 0;
        for (char c : str) {
            hash = hash * 31 + std::tolower(static_cast<unsigned char>(c));
        }
        return hash;
    }
};

struct HeaderEqual {
    bool operator()(const std::string& a, const std::string& b) const {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    }
};

using HeaderMap = std::unordered_map<std::string, std::string, HeaderHash, HeaderEqual>;

struct HTTPRequest {
    std::string method;
    std::string uri;
    std::string version;
    HeaderMap headers;
    std::string body;
};

struct HTTPResponse {
    std::string version;
    int status_code{0};
    std::string status_text;
    HeaderMap headers;
    std::string body;
};

class HTTPParser {
public:
    static HTTPRequest parse_request(const std::string& raw) {
        HTTPRequest req;
        std::istringstream stream(raw);
        std::string line;

        // 1. Request Line: METHOD URI VERSION
        if (std::getline(stream, line)) {
            trim_crlf(line);
            std::istringstream line_stream(line);
            line_stream >> req.method >> req.uri >> req.version;
        }

        // 2. Headers
        while (std::getline(stream, line)) {
            trim_crlf(line);
            if (line.empty()) break; // Empty line separates headers from body

            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                // Trim leading whitespace on value
                val.erase(0, val.find_first_not_of(" \t"));
                req.headers[key] = val;
            }
        }

        // 3. Body
        std::string body_data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        req.body = body_data;

        return req;
    }

    static HTTPResponse parse_response(const std::string& raw) {
        HTTPResponse resp;
        std::istringstream stream(raw);
        std::string line;

        // 1. Status Line: VERSION STATUS_CODE STATUS_TEXT
        if (std::getline(stream, line)) {
            trim_crlf(line);
            std::istringstream line_stream(line);
            line_stream >> resp.version >> resp.status_code;
            std::getline(line_stream, resp.status_text);
            resp.status_text.erase(0, resp.status_text.find_first_not_of(" \t"));
        }

        // 2. Headers
        while (std::getline(stream, line)) {
            trim_crlf(line);
            if (line.empty()) break;

            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                val.erase(0, val.find_first_not_of(" \t"));
                resp.headers[key] = val;
            }
        }

        // 3. Check for Chunked Transfer Encoding
        auto it = resp.headers.find("Transfer-Encoding");
        if (it != resp.headers.end() && it->second.find("chunked") != std::string::npos) {
            resp.body = decode_chunked_stream(stream);
        } else {
            std::string body_data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            resp.body = body_data;
        }

        return resp;
    }

private:
    static void trim_crlf(std::string& s) {
        if (!s.empty() && s.back() == '\r') s.pop_back();
    }

    static std::string decode_chunked_stream(std::istringstream& stream) {
        std::string decoded_body;
        std::string line;

        while (std::getline(stream, line)) {
            trim_crlf(line);
            if (line.empty()) continue;

            // Parse hex chunk size (e.g. "1a" or "0")
            size_t chunk_size = 0;
            std::stringstream ss;
            ss << std::hex << line;
            ss >> chunk_size;

            if (chunk_size == 0) {
                break; // Terminal chunk 0\r\n
            }

            // Read exact chunk_size bytes
            std::vector<char> chunk_buf(chunk_size);
            stream.read(chunk_buf.data(), chunk_size);
            decoded_body.append(chunk_buf.data(), chunk_size);

            // Read trailing CRLF after chunk data
            std::getline(stream, line);
        }

        return decoded_body;
    }
};

int main() {
    std::cout << "================ HTTP/1.1 PARSER & CHUNKED STREAMING VERIFICATION ================\n";

    // Test 1: HTTP Request Parsing
    std::string sample_req =
        "POST /api/v1/orders HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 26\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "{\"item\": \"apple\", \"qty\": 5}";

    auto req = HTTPParser::parse_request(sample_req);
    std::cout << "Parsed HTTP Request:\n";
    std::cout << "  Method:  " << req.method << "\n";
    std::cout << "  URI:     " << req.uri << "\n";
    std::cout << "  Version: " << req.version << "\n";
    std::cout << "  Host:    " << req.headers["host"] << " (Case-insensitive verification)\n";
    std::cout << "  Body:    \"" << req.body << "\"\n\n";

    assert(req.method == "POST");
    assert(req.uri == "/api/v1/orders");
    assert(req.headers["HOST"] == "api.example.com");

    // Test 2: Chunked HTTP Response Decoding
    std::string sample_chunked_resp =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "7\r\n"
        "Mozilla\r\n"
        "9\r\n"
        "Developer\r\n"
        "7\r\n"
        "Network\r\n"
        "0\r\n"
        "\r\n";

    auto resp = HTTPParser::parse_response(sample_chunked_resp);
    std::cout << "Parsed Chunked HTTP Response:\n";
    std::cout << "  Status:  " << resp.status_code << " " << resp.status_text << "\n";
    std::cout << "  Decoded Body (" << resp.body.size() << " bytes): \"" << resp.body << "\"\n";

    assert(resp.status_code == 200);
    assert(resp.body == "MozillaDeveloperNetwork");
    std::cout << "\nAll HTTP request parsing and chunked decoding verifications: PASSED!\n";
    std::cout << "===================================================================================\n";
    return 0;
}
