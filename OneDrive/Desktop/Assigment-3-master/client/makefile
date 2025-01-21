CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

# Default target
all: StompWCIClient

# Build StompWCIClient executable
StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o
	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o $(LDFLAGS)

# Build EchoClient executable (optional, if needed)
EchoClient: bin/ConnectionHandler.o bin/echoClient.o
	g++ -o bin/EchoClient bin/ConnectionHandler.o bin/echoClient.o $(LDFLAGS)

# Compile ConnectionHandler
bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

# Compile StompClient
bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

# Compile StompProtocol
bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

# Compile event (if used in the project)
bin/event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

# Compile echoClient (optional, if needed)
bin/echoClient.o: src/echoClient.cpp
	g++ $(CFLAGS) -o bin/echoClient.o src/echoClient.cpp

# Clean all compiled files
.PHONY: clean
clean:
	rm -f bin/*


# CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
# LDFLAGS:=-lboost_system -lpthread
# # TODO change the makefile for StompEMIlient
# all: StompWCIClient

# EchoClient: bin/ConnectionHandler.o bin/echoClient.o
# 	g++ -o bin/EchoClient bin/ConnectionHandler.o bin/echoClient.o $(LDFLAGS)

# StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/event.o
# 	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o $(LDFLAGS)

# bin/ConnectionHandler.o: src/ConnectionHandler.cpp
# 	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

# bin/echoClient.o: src/echoClient.cpp
# 	g++ $(CFLAGS) -o bin/echoClient.o src/echoClient.cpp

# bin/event.o: src/event.cpp
# 	g++ $(CFLAGS) -o bin/event.o src/event.cpp

# bin/StompClient.o: src/StompClient.cpp
# 	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

# .PHONY: clean
# clean:
# 	rm -f bin/*
	