#include "ConnectionHandler.h"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include "StompClient.h"
#include "StompProtocol.h"
#include <iostream>
#include <thread>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
 if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return -1;
    }
    
    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    cout << "Connecting to host: " << host << ", port: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << port << endl;

    StompProtocol protocol(host, port);
    cout << "pass protocol line" << endl;

    std::thread inputThread(&StompProtocol::handleUserInput, &protocol);
    std::thread serverThread(&StompProtocol::processServerMessages, &protocol);

    inputThread.join();
    serverThread.join();

	return 0;
}