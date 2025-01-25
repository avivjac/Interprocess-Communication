CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

# Default target
all: StompWCIClient

# Build StompWCIClient executable
StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o
	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o $(LDFLAGS)

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

# Clean all compiled files
.PHONY: clean
clean:
	rm -f bin/*


	