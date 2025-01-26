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

int main()
{

    // Atomic flag for thread management
    std::atomic<bool> running(true);
    bool isLogin = false;
    std::thread inputThread;
    std::thread serverThread;
    // bool isLogin = false;
    StompProtocol *protocol = nullptr;

    while (running)
    {
        string input;
        getline(cin, input);

        // Parse the user input into command and arguments
        istringstream iss(input);
        string command;
        iss >> command;

        if (command == "login")
        {
            if (!(protocol != nullptr && protocol->getIsConnected()))
            {

                string hostPort, username, password;
                iss >> hostPort >> username >> password;

                // Validate the format of host:port
                size_t colonPos = hostPort.find(':');
                if (colonPos == string::npos)
                {
                    cerr << "Invalid format for host:port. Expected <host>:<port>" << endl;
                    continue;
                }

                string host = hostPort.substr(0, colonPos);
                int port = stoi(hostPort.substr(colonPos + 1));

                protocol = new StompProtocol(host, port, username);

                // Attempt to connect
                if (!protocol->connect(username, password))
                {
                    cerr << "Login failed, please try again." << endl;

                    running = false;
                }
                else
                {
                    cout << "Login successful. Connected to " << host << ":" << port << endl;

                    if (!isLogin)
                    {
                        isLogin = true;
                        serverThread = std::thread(&StompProtocol::processServerMessages, protocol);
                        serverThread.detach();
                    }
                }
            }
            else
            {
                cerr << "Already logged in. Please logout first." << endl;
            }
        }
        else if (command == "join")
        {
            if ((protocol != nullptr && protocol->getIsConnected()))
            {
                string channel;
                iss >> channel;

                if (channel.empty())
                {
                    cerr << "Invalid channel name. Usage: join <channel>" << endl;
                }
                else
                {
                    protocol->subscribe(channel);
                }
            }
            else
            {
                cerr << "PLEASE LOG IN FIRST" << endl;
            }
        }
        else if (command == "exit")
        {
            if ((protocol != nullptr && protocol->getIsConnected()))
            {
                string channel;
                iss >> channel;

                if (channel.empty())
                {
                    cerr << "Invalid channel name. Usage: exit <channel>" << endl;
                }
                else
                {
                    protocol->unsubscribe(channel);
                }
            }
            else
            {
                cerr << "PLEASE LOG IN FIRST" << endl;
            }
        }
        else if (command == "logout")
        {
            if ((protocol != nullptr && protocol->getIsConnected()))
            {
                protocol->disconnect();
                if (serverThread.joinable())
                {
                    serverThread.join();
                }
                protocol = nullptr;
                isLogin = false;
            }
            else
            {
                cerr << "PLEASE LOG IN FIRST" << endl;
            }
        }
        else if (command == "report")
        {
            if ((protocol != nullptr && protocol->getIsConnected()))
            {
                string file;
                iss >> file;

                if (file.empty())
                {
                    cerr << "Invalid file name for report. Usage: report <file>" << endl;
                }
                else
                {
                    protocol->report(file);
                }
            }
            else
            {
                cerr << "PLEASE LOG IN FIRST" << endl;
            }
        }
        else if (command == "summary")
        {
            if ((protocol != nullptr && protocol->getIsConnected()))
            {
                string channelName, user, file;
                iss >> channelName >> user >> file;

                if (channelName.empty() || user.empty() || file.empty())
                {
                    cerr << "Invalid summary command. Usage: summary <channelName> <user> <file>" << endl;
                }
                else
                {
                    protocol->handleSummaryCommand(channelName, user, file);
                }
            }
            else
            {
                cerr << "PLEASE LOG IN FIRST" << endl;
            }
        }
        else
        {
            cerr << "Unknown command: " << command << endl;
        }
    }

    // Join threads if they were created
    if (protocol != nullptr)
    {
        serverThread.join();
        delete protocol;
    }

    cout << "Client terminated gracefully." << endl;
    return 0;
}
