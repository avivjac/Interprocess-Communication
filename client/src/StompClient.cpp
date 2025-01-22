#include "ConnectionHandler.h"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include "StompClient.h"
#include "StompProtocol.h"
#include <sstream>

using namespace std;

int main() {

    // Atomic flag for thread management
    std::atomic<bool> running(true);
     std::thread inputThread;
     std::thread serverThread;
    // bool isLogin = false;
    StompProtocol* protocol = nullptr;

   while (running)
     {
        string input;
        getline(cin, input);

        // Parse the user input into command and arguments
        istringstream iss(input);
        string command;
        iss >> command;

        if (command == "login") {
            string hostPort, username, password;
            iss >> hostPort >> username >> password;

            // Validate the format of host:port
            size_t colonPos = hostPort.find(':');
            if (colonPos == string::npos) {
                cerr << "Invalid format for host:port. Expected <host>:<port>" << endl;
                continue;
            }

            string host = hostPort.substr(0, colonPos);
            int port = stoi(hostPort.substr(colonPos + 1));

            protocol =new StompProtocol(host, port, username);
           

            // Attempt to connect
            if (!protocol->connect(username, password)) {
                cerr << "Login failed, please try again." << endl;
               
                running = false;
            } else {
                cout << "Login successful. Connected to " << host << ":" << port << endl;
                serverThread = std::thread(&StompProtocol::processServerMessages, protocol);
            }

        } 
         else if (command == "join") {
            string channel;
            iss >> channel;

            if (channel.empty()) {
                cerr << "Invalid channel name. Usage: join <channel>" << endl;
            } else {
                protocol->subscribe(channel);
                cout << "Subscribed to channel: " << channel << endl;
            }

        } else if (command == "exit") {
            string channel;
            iss >> channel;

            if (channel.empty()) {
                cerr << "Invalid channel name. Usage: exit <channel>" << endl;
            } else {
                protocol->unsubscribe(channel);
                cout << "Unsubscribed from channel: " << channel << endl;
            }

        } else if (command == "send") {
            string destination, message;
            iss >> destination;
            getline(iss, message);

            if (destination.empty() || message.empty()) {
                cerr << "Invalid send command. Usage: send <destination> <message>" << endl;
            } else {
                protocol->send(destination, message);
                cout << "Message sent to " << destination << ": " << message << endl;
            }

        } else if (command == "logout") {
           protocol->disconnect();
            cout << "Logged out successfully." << endl;

        } else if (command == "report") {
            string file;
            iss >> file;

            if (file.empty()) {
                cerr << "Invalid file name for report. Usage: report <file>" << endl;
            } else {
                protocol->report(file);
                cout << "Report generated from file: " << file << endl;
            }

        } else if (command == "summary") {
            string channelName, user, file;
            iss >> channelName >> user >> file;

            if (channelName.empty() || user.empty() || file.empty()) {
                cerr << "Invalid summary command. Usage: summary <channelName> <user> <file>" << endl;
            } else {
                protocol->handleSummaryCommand(channelName, user, file);
                cout << "Summary created for user " << user << " in channel " << channelName << " saved to " << file << endl;
            }

        } else {
            cerr << "Unknown command: " << command << endl;
        }

             
     }
    
// Join threads if they were created
    if (protocol != nullptr) {
        serverThread.join();
        delete protocol;
    }

    // Clean up protocol object after use
    



 

    
    // Initialize the StompProtocol object (empty for now, connection happens later)
    

    // Input thread for user commands
    // std::thread inputThread([&protocol, &running]() {
    //     try {
    //         protocol.handleUserInput();
    //     } catch (const std::exception &e) {
    //         cerr << "Error in input thread: " << e.what() << endl;
    //         running = false; // Stop the server thread
    //     }
    // });

    // Server thread for incoming messages
    // std::thread serverThread([&protocol, &running]() {
    //     try {
    //         protocol.processServerMessages();
    //     } catch (const std::exception &e) {
    //         cerr << "Error in server thread: " << e.what() << endl;
    //         running = false; // Stop the input thread
    //     }
    // });

    // Join threads
   

    
    cout << "Client terminated gracefully." << endl;
    return 0;
}
