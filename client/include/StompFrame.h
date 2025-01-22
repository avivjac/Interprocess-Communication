#pragma once

#ifndef STOMPFRAME_H
#define STOMPFRAME_H

#include <string>
#include <unordered_map>



class StompFrame {
private:
    std::string command;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

public:
    // Constructor
    StompFrame(const std::string& cmd, const std::unordered_map<std::string, std::string>& hdrs = {}, const std::string& bdy = "");

    // Getters
    const std::string& getCommand() const;
    const std::unordered_map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;

    // Add a header
    void addHeader(const std::string& key, const std::string& value);

    // Get a specific header
    std::string getHeader(const std::string& key) const;

    // Convert frame to string (serialization)
    std::string toString() const;

    // Parse a raw STOMP frame string (deserialization)
    static StompFrame parseFrame(const std::string& rawFrame);
};

#endif // STOMPFRAME_H
