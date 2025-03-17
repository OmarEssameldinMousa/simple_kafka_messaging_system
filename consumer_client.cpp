// consumer_client.cpp
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <topic> [partition]\n";
        return 1;
    }
    std::string topic = argv[1];
    int partition = 0;
    if (argc >= 3) {
        partition = std::atoi(argv[2]);
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket.\n";
        return 1;
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address.\n";
        return 1;
    }
    memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed.\n";
        return 1;
    }

    char buffer[BUFFER_SIZE];
    while (true) {
        std::ostringstream oss;
        oss << "CONSUME " << topic << " " << partition;
        std::string command = oss.str();
        if (send(clientSocket, command.c_str(), command.size(), 0) < 0) {
            std::cerr << "Send failed.\n";
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = read(clientSocket, buffer, BUFFER_SIZE - 1);
        if (bytesRead <= 0) {
            std::cerr << "Connection closed or read error.\n";
            break;
        }
        std::cout << "Server response: " << buffer;
    }
    close(clientSocket);
    return 0;
}
