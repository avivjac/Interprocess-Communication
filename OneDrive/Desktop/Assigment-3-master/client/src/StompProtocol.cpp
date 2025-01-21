#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <fstream>
#include "StompProtocol.h"
#include "event.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <iomanip>

using namespace std;



StompProtocol::StompProtocol(const string& host, int port): connectionHandler(host, port), isConnected(false), subscriptionCounter(0) {    
    cout << "counstructorrrrrrrrr" << endl;
    if (connectionHandler.connect()) {
        isConnected = true;
    } else {
        std::cerr << "Could not connect to server" << std::endl;
    }
}

bool StompProtocol::connect(const string& username, const string& password) {
    if (!connectionHandler.connect()) {
        cerr << "Failed to connect to server" << endl;
        return false;
    }

    string connectFrame = "CONNECT\naccept-version:1.2\nlogin:" + username + "\npasscode:" + password + "\n\n\0";
    if (!connectionHandler.sendFrameAscii(connectFrame, '\0')) {
        cerr << "Failed to send CONNECT frame" << endl;
        return false;
    }

    this->username = username;
    isConnected = true;
    return true;
}

void StompProtocol::saveEvents(const std::string& channel, const std::vector<Event>& events) {
    for (const Event& event : events) {
    savedEvents[channel].push_back(event); // `savedEvents` is a `std::map<std::string, std::vector<Event>>`
}
}

void StompProtocol::disconnect() {
    if (isConnected) {
        string disconnectFrame = "DISCONNECT\n\n\0";
        connectionHandler.sendFrameAscii(disconnectFrame, '\0');
        connectionHandler.close();
        isConnected = false;
    }
}

void StompProtocol::subscribe(const string& channel) {
    int subscriptionId = ++subscriptionCounter;
    subscriptions[channel] = subscriptionId;

    string subscribeFrame = "SUBSCRIBE\nid:" + to_string(subscriptionId) +
                            "\ndestination:" + channel + "\n\n\0";
    connectionHandler.sendFrameAscii(subscribeFrame, '\0');
}

void StompProtocol::unsubscribe(const string& channel) {
    if (subscriptions.find(channel) != subscriptions.end()) {
        int subscriptionId = subscriptions[channel];
        subscriptions.erase(channel);

        string unsubscribeFrame = "UNSUBSCRIBE\nid:" + to_string(subscriptionId) + "\n\n\0";
        connectionHandler.sendFrameAscii(unsubscribeFrame, '\0');
    }
}

//sketchy - do we need to parse the message? we recieve json path
void StompProtocol::send(const string& destination, const string& message) {
    string sendFrame = "SEND\ndestination:" + destination + "\n\n" + message + "\0";
    connectionHandler.sendFrameAscii(sendFrame, '\0');
}

void StompProtocol::processServerMessages() {
    while (isConnected) {
        string frame;
        if (connectionHandler.getFrameAscii(frame, '\0')) {
            cout << "Server: " << frame << endl;
        } else {
            cerr << "Disconnected from server" << endl;
            isConnected = false;
        }
    }
}

void StompProtocol::report(const std::string& file) {
    names_and_events parsedData = parseEventsFile(file);
    std::string channelName = parsedData.channel_name;
    std::vector<Event> events = parsedData.events;

    // Sort events by date_time (optional, if not already sorted)
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.get_date_time() < b.get_date_time();
    });

    // Iterate through the events and send each as a STOMP SEND frame
    for (const Event& event : events) {
        sendEventToChannel(channelName, event);
    }

    // Save the events for summarization (optional)
    saveEvents(channelName, events);
}

void StompProtocol::sendEventToChannel(const std::string& channel, const Event& event) {
    std::ostringstream frameBody;

    // Extract general information from the map
    auto generalInfo = event.get_general_information(); // Assuming this returns a std::map<std::string, std::string>
    bool active = (generalInfo["active"] == "true");
    bool forces_arrival_at_scene = (generalInfo["forces_arrival_at_scene"] == "true");

    // Construct the frame body
    frameBody << "user:" << username << "\n"
              << "city:" << event.get_city() << "\n"
              << "event name:" << event.get_name() << "\n"
              << "date time:" << event.get_date_time() << "\n"
              << "general information:" << "\n"
              << "active:" << (active ? "true" : "false") << "\n"
              << "forces_arrival_at_scene:" << (forces_arrival_at_scene ? "true" : "false") << "\n"
              << "description:" << event.get_description() << "\n";

    // Send the constructed frame body to the channel
    send(channel, frameBody.str());
}


void StompProtocol::handleUserInput() {
    string input;
    while (isConnected) {
        getline(cin, input);

        istringstream iss(input);
        string command;
        iss >> command;

        if (command == "login") {
            string hostPort, username, password;
            iss >> hostPort >> username >> password;

            string host = hostPort.substr(0, hostPort.find(':'));
            int port = stoi(hostPort.substr(hostPort.find(':') + 1));

            if (!connect(username, password)) {
                cerr << "Login failed" << endl;
            }
        } else if (command == "join") {
            string channel;
            iss >> channel;
            subscribe(channel);
        } else if (command == "exit") {
            string channel;
            iss >> channel;
            unsubscribe(channel);
        } else if (command == "send") {
            string destination, message;
            iss >> destination;
            getline(iss, message);
            send(destination, message);
        } else if (command == "logout") {
            disconnect();
            break;
        } else if (command == "report") {
            std::string file;
            iss >> file;
            report(file);
        } else if (command == "summary") {
            string channelName, user, file;
            iss >> channelName >> user >> file;
            handleSummaryCommand(channelName, user, file);
        }
        else {
            cerr << "Unknown command: " << command << endl;
        }
    }
}

void StompProtocol::handleSummaryCommand(const std::string& channelName, const std::string& user, const std::string& file) {
    // Retrieve emergency updates for the specified channel and user
    vector<Event> events = savedEvents[channelName];

    // Summarize statistics
    int totalReports = events.size();
    int activeCount = std::count_if(events.begin(), events.end(), [](const Event& update) { auto generalInfo = update.get_general_information(); 
                                                                                            return (generalInfo["active"] == "true"); });
    int forcesArrivalCount = std::count_if(events.begin(), events.end(), [](const Event& update) { auto generalInfo = update.get_general_information(); 
                                                                                            return (generalInfo["forces_arrival_at_scene"] == "true"); });

    // Sort reports by date_time and event_name
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        if (a.get_date_time() != b.get_date_time()) {
            return a.get_date_time() < b.get_date_time();
        }
        return a.get_name() < b.get_name();
    });

    // Write summary to file
    std::ofstream outFile(file);
    if (!outFile.is_open()) {
        std::cerr << "Could not open file: " << file << std::endl;
        return;
    }

    outFile << "Channel " << channelName << "\n";
    outFile << "Stats:\n";
    outFile << "Total: " << totalReports << "\n";
    outFile << "active: " << activeCount << "\n";
    outFile << "forces arrival at scene: " << forcesArrivalCount << "\n";
    outFile << "Event Reports:\n";

    for (const auto& update : events) {
        outFile << "Report:\n";
        outFile << "city: " << update.get_city() << "\n";
        outFile << "date time: " << epochToDate(update.get_date_time()) << "\n";
        outFile << "event name: " << update.get_name() << "\n";
        outFile << "summary: " << summarizeDescription(update.get_description()) << "\n";
    }

    outFile.close();
}

std::string StompProtocol::epochToDate(time_t epochTime) {
    std::tm* tm = std::localtime(&epochTime);
    std::ostringstream oss;
    oss << std::put_time(tm, "%d/%m/%y %H:%M");
    return oss.str();
}

std::string StompProtocol::summarizeDescription(const std::string& description) {
    if (description.length() <= 27) {
        return description;
    }
    return description.substr(0, 27) + "...";
}

