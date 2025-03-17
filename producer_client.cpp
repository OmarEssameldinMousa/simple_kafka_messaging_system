// producer_client.cpp
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <thread>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <topic> <mode(single|stream)>\n";
        return 1;
    }
    std::string topic = argv[1];
    std::string mode = argv[2];

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
    int msgId = 1;
    if (mode == "single") {
        std::string msgContent;
        std::cout << "Enter message: ";
        std::getline(std::cin, msgContent);
        std::ostringstream oss;
        oss << "PRODUCE " << topic << " " << msgId << " " << msgContent;
        std::string command = oss.str();
        if (send(clientSocket, command.c_str(), command.size(), 0) < 0) {
            std::cerr << "Send failed.\n";
        }
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = read(clientSocket, buffer, BUFFER_SIZE - 1);
        if (bytesRead > 0) {
            std::cout << "Server response: " << buffer;
        }
    } else if (mode == "stream") {
        while (true) {
            std::ostringstream oss;
            oss << "PRODUCE " << topic << " " << msgId << " " << "Log message " << msgId;
            std::string command = oss.str();
            if (send(clientSocket, command.c_str(), command.size(), 0) < 0) {
                std::cerr << "Send failed.\n";
                break;
            }
            memset(buffer, 0, BUFFER_SIZE);
            int bytesRead = read(clientSocket, buffer, BUFFER_SIZE - 1);
            if (bytesRead > 0) {
                std::cout << "Server response: " << buffer;
            }
            msgId++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        std::cerr << "Invalid mode. Use 'single' or 'stream'.\n";
    }
    close(clientSocket);
    return 0;
}
