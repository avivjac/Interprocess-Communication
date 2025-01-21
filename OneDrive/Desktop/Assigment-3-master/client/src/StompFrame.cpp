#include "StompFrame.h"
#include <sstream>

// Constructor
StompFrame::StompFrame(const std::string& cmd, const std::unordered_map<std::string, std::string>& hdrs, const std::string& bdy)
    : command(cmd), headers(hdrs), body(bdy) {}

// Getters
const std::string& StompFrame::getCommand() const {
    return command;
}

const std::unordered_map<std::string, std::string>& StompFrame::getHeaders() const {
    return headers;
}

const std::string& StompFrame::getBody() const {
    return body;
}

// Add a header
void StompFrame::addHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

// Get a specific header
std::string StompFrame::getHeader(const std::string& key) const {
    auto it = headers.find(key);
    return (it != headers.end()) ? it->second : "";
}

// Convert frame to string (serialization)
std::string StompFrame::toString() const {
    std::ostringstream frame;

    // Add the command
    frame << command << "\n";

    // Add the headers
    for (const auto& header : headers) {
        frame << header.first << ":" << header.second << "\n";
    }

    // Add a blank line and the body
    frame << "\n" << body;

    return frame.str();
}

// Parse a raw STOMP frame string (deserialization)
StompFrame StompFrame::parseFrame(const std::string& rawFrame) {
    std::istringstream stream(rawFrame);
    std::string line;

    // Read command
    std::getline(stream, line);
    std::string cmd = line;

    // Read headers
    std::unordered_map<std::string, std::string> hdrs;
    while (std::getline(stream, line) && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            hdrs[key] = value;
        }
    }

    // Read body
    std::string bdy;
    std::string bodyLine;
    while (std::getline(stream, bodyLine)) {
        bdy += bodyLine + "\n";
    }

    // Trim trailing newline from the body
    if (!bdy.empty() && bdy.back() == '\n') {
        bdy.pop_back();
    }

    return StompFrame(cmd, hdrs, bdy);
}
