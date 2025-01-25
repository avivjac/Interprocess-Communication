#pragma once

#include "ConnectionHandler.h"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <map>

class StompClient
{
private:
    ConnectionHandler connectionHandler;
    std::atomic<bool> isConnected;
    std::map<std::string, int> subscriptions; // Map channel names to subscription IDs
    int subscriptionCounter;
    std::string username;

public:
    StompClient(const std::string &host, int port, const std::string &username) : connectionHandler(host, port), isConnected(false), subscriptions(), subscriptionCounter(0), username(username) {}
};
