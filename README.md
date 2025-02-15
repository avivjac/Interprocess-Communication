# Interprocess-Communication
Overview

This project implements a communication service using the STOMP protocol. It consists of a server written in Java and a client written in C++. The server supports both Thread-Per-Client and Reactor models for handling multiple connections efficiently.

Features

STOMP Protocol Support: Implements STOMP for message exchange.

Multi-threading: Supports Thread-Per-Client and Reactor concurrency models.

Client-Server Architecture: Java-based server and C++ client.

User Authentication: Handles login and logout functionality.

Message Routing: Manages user subscriptions and message delivery.

Technologies Used

Java (for the server)

C++ (for the client)

STOMP protocol

Multi-threading

Installation

Server

Clone the repository:

git clone <repository_url> cd communication_project/server

Build the project using Maven:

mvn clean install

Run the server:

java -jar target/server.jar

Client

Navigate to the client directory:

cd communication_project/client

Compile the client:

g++ -std=c++11 -o client main.cpp StompClient.cpp StompProtocol.cpp -lpthread

Run the client:

./client

Usage

Start the server.

Run the client and use the following commands:

login :

join

send

exit

logout
