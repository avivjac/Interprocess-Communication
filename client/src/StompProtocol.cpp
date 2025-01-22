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



// StompProtocol::StompProtocol(const string& host, int port): connectionHandler(host, port), isConnected(false),subscriptions() ,subscriptionCounter(0),username(),savedEvents() {    
//     cout << "counstructorrrrrrrrr" << endl;
//     if (connectionHandler.connect()) {
//         isConnected = true;
//     } else {
//         std::cerr << "Could not connect to server" << std::endl;
//     }
// }

StompProtocol::StompProtocol(const string &host, int port, const string &username):connectionHandler(host, port), isConnected(false),subscriptions() ,subscriptionCounter(0),username(username),savedEvents()
{

}

// StompProtocol::StompProtocol(): connectionHandler("", 0), isConnected(false),subscriptions() ,subscriptionCounter(0),username(),savedEvents()
// {}

// void StompProtocol::updateStompProtocol(const string& host, int port) {
//     connectionHandler.updateConnectionHandler(host, port);
//     isConnected = false;
//     subscriptions.clear();
//     subscriptionCounter = 0;
//     username = "";
//     savedEvents.clear();
// }

bool StompProtocol::connect(const std::string &username, const std::string &password) {
    // Ensure connection handler is initialized
    if (!connectionHandler.connect()) {
        std::cerr << "Failed to connect to server." << std::endl;
        isConnected = false;
        return false;
    }
    else
    {
        cout << "connected to server" << endl;
        isConnected = true;
    }

    // Construct and send CONNECT frame
    std::string connectFrame = "CONNECT\naccept-version:1.2\nlogin:" + username + "\npasscode:" + password + "\n\n\0";
    if (!connectionHandler.sendFrameAscii(connectFrame, '\0')) {
        std::cerr << "Failed to send CONNECT frame." << std::endl;
        return false;
    }

    // Await server response (CONNECTED frame)
    std::string response;
    if (!connectionHandler.getFrameAscii(response, '\0')) {
        std::cerr << "Failed to receive CONNECTED frame." << std::endl;
        return false;
    }

    // Validate response
    if (response.find("CONNECTED") != std::string::npos) {
        isConnected = true;
        return true;
    }

    std::cerr << "Unexpected server response: " << response << std::endl;
    return false;
}


void StompProtocol::saveEvents(const std::string& channel, const std::vector<Event>& events) {
    for (const Event& event : events) {
    savedEvents[channel].push_back(event); // `savedEvents` is a `std::map<std::string, std::vector<Event>>`
}
}

void StompProtocol::disconnect() {
    if (!isConnected) {
        std::cerr << "Not connected to any server." << std::endl;
        return;
    }

    // Send DISCONNECT frame
    std::string disconnectFrame = "DISCONNECT\n\n\0";
    connectionHandler.sendFrameAscii(disconnectFrame, '\0');

    // Close the connection
    connectionHandler.close();
    //////////////////////////////////isConnected = false;
}

void StompProtocol::subscribe(const std::string &channel) {
    int subscriptionId = ++subscriptionCounter; // Generate a unique subscription ID
    subscriptions[channel] = subscriptionId;

    // Construct and send SUBSCRIBE frame
    std::string subscribeFrame = "SUBSCRIBE\ndestination:" + channel +
                                 "\nid:" + std::to_string(subscriptionId) + "\n\n\0";
    connectionHandler.sendFrameAscii(subscribeFrame, '\0');
}

void StompProtocol::unsubscribe(const std::string &channel) {
    if (subscriptions.find(channel) == subscriptions.end()) {
        std::cerr << "Not subscribed to channel: " << channel << std::endl;
        return;
    }

    int subscriptionId = subscriptions[channel];
    subscriptions.erase(channel);

    // Construct and send UNSUBSCRIBE frame
    std::string unsubscribeFrame = "UNSUBSCRIBE\nid:" + std::to_string(subscriptionId) + "\n\n\0";
    connectionHandler.sendFrameAscii(unsubscribeFrame, '\0');
}


//sketchy - do we need to parse the message? we recieve json path
void StompProtocol::send(const std::string &destination, const std::string &message) {
    // Construct and send SEND frame
    std::string sendFrame = "SEND\ndestination:" + destination + "\n\n" + message + "\0";
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

void StompProtocol::report(const std::string &file) {
    // Open the file and read its contents
    std::ifstream inputFile(file);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file: " << file << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string fileContent = buffer.str();

    // Construct and send a SEND frame with the report
    send("/report", fileContent);
    std::cout << "Report sent for file: " << file << std::endl;
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


// void StompProtocol::handleUserInput() {
//     string input;

//     while (isConnected) {
//         getline(cin, input);

//         // Parse the user input into command and arguments
//         istringstream iss(input);
//         string command;
//         iss >> command;

//         if (command == "login") {
//             string hostPort, username, password;
//             iss >> hostPort >> username >> password;

//             // Validate the format of host:port
//             size_t colonPos = hostPort.find(':');
//             if (colonPos == string::npos) {
//                 cerr << "Invalid format for host:port. Expected <host>:<port>" << endl;
//                 continue;
//             }

//             string host = hostPort.substr(0, colonPos);
//             int port = stoi(hostPort.substr(colonPos + 1));

//             StompProtocol protocol(host, port,username);
//             //  std::thread inputThread(&StompProtocol::handleUserInput, &protocol);
//             // std::thread serverThread(&StompProtocol::processServerMessages, &protocol);

//             // Attempt to connect
//             if (!connect(username, password)) {
//                 cerr << "Login failed, please try again." << endl;
//             } else {
//                 cout << "Login successful. Connected to " << host << ":" << port << endl;
//             }

//         }
        
//          else if (command == "join") {
//             string channel;
//             iss >> channel;

//             if (channel.empty()) {
//                 cerr << "Invalid channel name. Usage: join <channel>" << endl;
//             } else {
//                 subscribe(channel);
//                 cout << "Subscribed to channel: " << channel << endl;
//             }

//         } else if (command == "exit") {
//             string channel;
//             iss >> channel;

//             if (channel.empty()) {
//                 cerr << "Invalid channel name. Usage: exit <channel>" << endl;
//             } else {
//                 unsubscribe(channel);
//                 cout << "Unsubscribed from channel: " << channel << endl;
//             }

//         } else if (command == "send") {
//             string destination, message;
//             iss >> destination;
//             getline(iss, message);

//             if (destination.empty() || message.empty()) {
//                 cerr << "Invalid send command. Usage: send <destination> <message>" << endl;
//             } else {
//                 send(destination, message);
//                 cout << "Message sent to " << destination << ": " << message << endl;
//             }

//         } else if (command == "logout") {
//             disconnect();
//             isConnected = false;
//             cout << "Logged out successfully." << endl;
//             break;

//         } else if (command == "report") {
//             string file;
//             iss >> file;

//             if (file.empty()) {
//                 cerr << "Invalid file name for report. Usage: report <file>" << endl;
//             } else {
//                 report(file);
//                 cout << "Report generated from file: " << file << endl;
//             }

//         } else if (command == "summary") {
//             string channelName, user, file;
//             iss >> channelName >> user >> file;

//             if (channelName.empty() || user.empty() || file.empty()) {
//                 cerr << "Invalid summary command. Usage: summary <channelName> <user> <file>" << endl;
//             } else {
//                 handleSummaryCommand(channelName, user, file);
//                 cout << "Summary created for user " << user << " in channel " << channelName << " saved to " << file << endl;
//             }

//         } else {
//             cerr << "Unknown command: " << command << endl;
//         }
//     }
    
// }



void StompProtocol::handleSummaryCommand(const std::string &channelName, const std::string &user, const std::string &file) {
    // Construct the summary based on channelName and user
    std::string summary = "Summary for user: " + user + " in channel: " + channelName;

    // Write the summary to the specified file
    std::ofstream outputFile(file);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << file << std::endl;
        return;
    }

    outputFile << summary;
    outputFile.close();
    std::cout << "Summary written to file: " << file << std::endl;
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

