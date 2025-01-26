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
#include <mutex>
#include <algorithm>
#include <mutex>
#include <regex>

using namespace std;

StompProtocol::StompProtocol(const string &host, int port, const string &username) : connectionHandler(host, port), isConnected(false), subscriptions(), subscriptionCounter(0), receiptCounter(0), logOutid(-999), username(username), savedEvents(), connectionMutex(), subscriptionsMutex(), eventMutex()
{
}

bool StompProtocol::connect(const std::string &username, const std::string &password)
{
    std::lock_guard<std::mutex> lock(connectionMutex);
    // Ensure connection handler is initialized
    if (!connectionHandler.connect())
    {
        isConnected = false;
        return false;
    }
    else
    {
        isConnected = true;
    }

    // Construct and send CONNECT frame
    std::string connectFrame = "CONNECT\naccept-version:1.2\nlogin:" + username + "\npasscode:" + password + "\n\n\0";
    if (!connectionHandler.sendFrameAscii(connectFrame, '\0'))
    {
        std::cerr << "Failed to send CONNECT frame." << std::endl;
        return false;
    }
    return true;

}

void StompProtocol::saveEvents(const std::string &channel, const std::vector<Event> &events)
{
    for (const Event &event : events)
    {
        savedEvents[channel].push_back(event); // `savedEvents` is a `std::map<std::string, std::vector<Event>>`
    }
}

void StompProtocol::disconnect()
{
    std::lock_guard<std::mutex> lock(connectionMutex);

    if (!isConnected)
    {
        std::cerr << "Not connected to any server." << std::endl;
        return;
    }

    isConnected = false;
    logOutid = receiptCounter; // Generate a unique receipt ID
    receiptCounter++;

    // Send DISCONNECT frame
    std::string disconnectFrame = "DISCONNECT\nreceipt:" + std::to_string(logOutid) + "\n\n\0";
    connectionHandler.sendFrameAscii(disconnectFrame, '\0');
}

void StompProtocol::subscribe(const std::string &channel)
{
    int subscriptionId = ++subscriptionCounter; // Generate a unique subscription ID
    std::lock_guard<std::mutex> subLock(subscriptionsMutex);
    subscriptions[channel] = subscriptionId;

    // Construct and send SUBSCRIBE frame
    std::string subscribeFrame = "SUBSCRIBE\ndestination:" + channel +
                                 "\nid:" + std::to_string(subscriptionId) +
                                 "\nreceipt:" + std::to_string(receiptCounter) + "\n\n\0";
    receiptCounter++;
    connectionHandler.sendFrameAscii(subscribeFrame, '\0');
}

void StompProtocol::unsubscribe(const std::string &channel)
{
    std::lock_guard<std::mutex> subLock(subscriptionsMutex);
    if (subscriptions.find(channel) == subscriptions.end())
    {
        std::cerr << "Not subscribed to channel: " << channel << std::endl;
        return;
    }

    int subscriptionId = subscriptions[channel];
    subscriptions.erase(channel);

    // Construct and send UNSUBSCRIBE frame
    std::string unsubscribeFrame = "UNSUBSCRIBE\nid:" + std::to_string(subscriptionId) + "\nreceipt:" + std::to_string(receiptCounter) + "\n\n\0";
    receiptCounter++;
    connectionHandler.sendFrameAscii(unsubscribeFrame, '\0');
}

void StompProtocol::send(const std::string &destination, const std::string &message)
{
    // Construct and send SEND frame
    std::string sendFrame = "SEND\ndestination:" + destination + "\n\n" + message + "\0";
    connectionHandler.sendFrameAscii(sendFrame, '\0');
}

void StompProtocol::processServerMessages()
{

    while (isConnected)
    {

        std::string message;

        if (!connectionHandler.getFrameAscii(message, '\0'))
        {
            std::cerr << "Disconnected from server." << std::endl;
            isConnected = false;
        }

        // Parse the server message
        if (message.find("CONNECTED") != std::string::npos)
        {
            // Handle CONNECTED frame
            std::cout << "Successfully connected to the server." << std::endl;
            std::cout << message << std::endl;
        }
        else if (message.find("RECEIPT") != std::string::npos)
        {
            // Handle RECEIPT frame
            std::cout << message << std::endl;

            // handle Disconnect receipt
            if (message.find("receipt-id:" + std::to_string(logOutid)) != std::string::npos)
            {
                connectionHandler.close(); // close the connection
                isConnected = false;
            }
        }
        else if (message.find("ERROR") != std::string::npos)
        {
            // Handle ERROR frame
            std::cerr << message << std::endl;
            disconnect(); // do we need to disconnect here?
        }
        else if (message.find("MESSAGE") != std::string::npos)
        {
            // Handle MESSAGE frame (e.g., event reports)
            Event event = parseEventMessage(message); // Parse the event from the message
            std::string channel = event.get_channel_name();

            if (savedEvents.find(channel) == savedEvents.end())
            {
                savedEvents[channel] = std::vector<Event>();
            }
            savedEvents[channel].push_back(event);

            // Sort the events by date_time
            std::sort(savedEvents[channel].begin(), savedEvents[channel].end(), [](const Event &a, const Event &b)
                      {
                          return a.get_date_time() < b.get_date_time(); // Sort by date_time
                      });

            // convert the epoch time to date time inside the message
            regex dateRegex("date time:([0-9]+)"); // Matches "date time:" followed by one or more digits
            smatch match;

            // Search for the first occurrence of "date time:" in the message
            if (regex_search(message, match, dateRegex))
            {
                string epochString = match[1]; // Extract the epoch time as a string

                // Convert the string to an integer
                int epochTime = stoi(epochString);
                // Convert epoch to a readable date-time string
                string dateTime = epochToDate(epochTime);

                // Replace the epoch timestamp in the original message with the date-time string
                message.replace(message.find(epochString), epochString.length(), dateTime);
            }

            std::cout << message << std::endl;
        }
        else
        {
            // Handle unknown or unexpected messages
            std::cout << "Unknown server message:\n"
                      << message << std::endl;
        }
    }
}

Event StompProtocol::parseEventMessage(const std::string &message)
{
    // Variables to store parsed values
    std::string city, name, description, channel;
    std::time_t date_time = 0;
    std::map<std::string, std::string> general_information;

    // Helper function to extract fields
    auto extractField = [](const std::string &msg, size_t startPos, const std::string &fieldName) -> std::string
    {
        size_t fieldStart = msg.find(fieldName, startPos) + fieldName.size();
        size_t fieldEnd = msg.find('\n', fieldStart);
        return msg.substr(fieldStart, fieldEnd - fieldStart);
    };

    // Parse city
    size_t cityPos = message.find("city:");
    if (cityPos != std::string::npos)
    {
        city = extractField(message, cityPos, "city:");
    }

    // Parse name
    size_t namePos = message.find("event name:");
    if (namePos != std::string::npos)
    {
        name = extractField(message, namePos, "event name:");
    }

    // Parse description
    size_t descriptionPos = message.find("description:");
    if (descriptionPos != std::string::npos)
    {
        description = extractField(message, descriptionPos, "description:");
    }

    // Parse date_time
    size_t dateTimePos = message.find("date time:");
    if (dateTimePos != std::string::npos)
    {
        std::string dateTimeStr = extractField(message, dateTimePos, "date time:");
        date_time = std::stol(dateTimeStr); // Convert to epoch time
    }

    // Parse general information (if applicable)
    size_t generalInfoPos = message.find("general information:");
    if (generalInfoPos != std::string::npos)
    {
        size_t generalInfoEnd = message.find("\n\n", generalInfoPos); // End of general info section
        std::string generalInfoBlock = message.substr(generalInfoPos + std::string("general information:").size(),
                                                      generalInfoEnd - generalInfoPos);

        std::istringstream generalInfoStream(generalInfoBlock);
        std::string line;
        while (std::getline(generalInfoStream, line))
        {
            size_t delimiterPos = line.find(':');
            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                general_information[key] = value;
            }
        }
    }

    // Parse channel (if applicable)
    size_t channelPos = message.find("destination:");
    if (channelPos != std::string::npos)
    {
        channel = extractField(message, channelPos, "destination:");
    }

    Event e(channel, city, name, date_time, description, general_information);
    e.setEventOwnerUser(extractField(message, 0, "user:"));

    return e;
}

void StompProtocol::report(const std::string &filePath)
{

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    names_and_events parsedEvents = parseEventsFile(filePath);

    if (subscriptions.find(parsedEvents.channel_name) == subscriptions.end())
    {
        std::cerr << "Didn't subscribe to channel " + parsedEvents.channel_name << std::endl;
        return;
    }
    if (parsedEvents.channel_name.empty())
    {
        std::cerr << "No channel name found in file: " << filePath << std::endl;
        return;
    }
    if (parsedEvents.events.empty())
    {
        std::cerr << "No events found in file: " << filePath << std::endl;
        return;
    }

    for (Event &event : parsedEvents.events)
    {
        sendEventToChannel(parsedEvents.channel_name, event);
    }
}

void StompProtocol::sendEventToChannel(const std::string &channel, const Event &event)
{
    std::lock_guard<std::mutex> lock(eventMutex);
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

void StompProtocol::handleSummaryCommand(const std::string &channelName, const std::string &user, const std::string &file)
{
    std::lock_guard<std::mutex> lock(eventMutex);

    if (subscriptions.find(channelName) == subscriptions.end())
    {
        std::cerr << "Not subscribed to channel: " << channelName << std::endl;
        return;
    }

    // Open the output file
    std::ofstream outputFile(file);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open file for writing: " << file << std::endl;
        return;
    }

    cout << "Summary created for user " << user << " in channel " << channelName << " saved to " << file << endl;

    // Header: Channel and Stats
    outputFile << "Channel < " << channelName << " >\n";
    outputFile << "Stats :\n";

    // Calculate stats
    int totalReports = 0;
    int activeCount = 0;
    int forcesArrivalCount = 0;
    // Iterate over events for the given channel
    for (Event &event : savedEvents[channelName])
    {

        if (event.getEventOwnerUser() == user)
        {
            totalReports++;
            map<std::string, std::string> generalinfo = event.get_general_information();
            bool active = (generalinfo["active"] == "true");
            bool forces_arrival_at_scene = (generalinfo["forces_arrival_at_scene"] == "true");

            if (active)
            { // Assuming `get_active()` returns a boolean
                activeCount++;
            }
            if (forces_arrival_at_scene)
            { // Assuming `get_forces_arrival_at_scene()` returns a boolean
                forcesArrivalCount++;
            }
        }
    }

    // Write stats to the file
    outputFile << "Total : " << totalReports << "\n";
    outputFile << "active : " << activeCount << "\n";
    outputFile << "forces arrival at scene : " << forcesArrivalCount << "\n";

    // Event Reports Section
    outputFile << "Event Reports :\n";

    int reportCounter = 1;
    for (const Event &event : savedEvents[channelName])
    {
        if (event.getEventOwnerUser() == user)
        { // Include only events matching the user's name
            outputFile << "Report_" << reportCounter++ << " :\n";
            outputFile << "city : " << event.get_city() << "\n"; // Assuming `get_city()` exists
            outputFile << "date time : " << epochToDate(event.get_date_time()) << "\n";
            outputFile << "event name : " << event.get_name() << "\n";
            outputFile << "summary : " << summarizeDescription(event.get_description()) << "\n\n";
        }
    }

    outputFile.close(); // Close the file
}

std::string StompProtocol::extractField(const std::string &message, size_t startPos, const std::string &fieldName)
{
    size_t endPos = message.find('\n', startPos);
    if (endPos == std::string::npos)
    {
        endPos = message.length();
    }
    return message.substr(startPos + fieldName.length(), endPos - startPos - fieldName.length());
}

std::string StompProtocol::epochToDate(time_t epochTime)
{
    std::tm *tm = std::localtime(&epochTime);
    std::ostringstream oss;
    oss << std::put_time(tm, "%d/%m/%y %H:%M");
    return oss.str();
}

std::string StompProtocol::summarizeDescription(const std::string &description)
{
    if (description.length() <= 27)
    {
        return description;
    }
    return description.substr(0, 27) + "...";
}

// getter
bool StompProtocol::getIsConnected()
{
    return isConnected;
}
