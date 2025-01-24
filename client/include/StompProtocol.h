#pragma once

#include "../include/ConnectionHandler.h"
#include "event.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

class StompProtocol
{
private:
    ConnectionHandler connectionHandler;
    bool isConnected;
    std::map<std::string, int> subscriptions; // Map channel names to subscription IDs
    int subscriptionCounter;
    int receiptCounter;
    int logOutid;
    std::string username;
    std::map<std::string, std::vector<Event>> savedEvents; // Map channel names to saved events
    std::mutex connectionMutex;
    std::mutex subscriptionsMutex;
    std::mutex eventMutex;
public:
    StompProtocol(const string &host, int port);
    StompProtocol(const string &host, int port, const string &username);
    StompProtocol();
    void updateStompProtocol(const string &host, int port);
    bool connect(const string &username, const string &password);
    void saveEvents(const std::string &channel, const std::vector<Event> &events);
    void disconnect();
    void subscribe(const string &channel);
    void unsubscribe(const string &channel);
    void send(const string &destination, const string &message);
    void processServerMessages();
    Event parseEventMessage(const std::string &message);
    void handleUserInput();
    void report(const std::string &file);
    void sendEventToChannel(const std::string &channel, const Event &event);
    void handleSummaryCommand(const std::string &channelName, const std::string &user, const std::string &file);
    std::string extractField(const std::string &message, size_t startPos, const std::string &fieldName);
    string epochToDate(time_t epochTime);
    string summarizeDescription(const string &description);
    bool getIsConnected();
};
