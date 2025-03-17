// broker_server.cpp
#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <string>

#define PORT 8080
#define BUFFER_SIZE 1024

// Message structure
struct Message {
    int id;
    std::string content;
    Message(int _id, const std::string& _content)
        : id(_id), content(_content) {}
};

// Partition class to store messages in a thread-safe way
class Partition {
private:
    std::queue<Message> messages;
    std::mutex mtx;
    std::condition_variable cv;
public:
    void addMessage(const Message& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        messages.push(msg);
        cv.notify_one();
    }
    // This call blocks until a message is available.
    Message getMessage() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this](){ return !messages.empty(); });
        Message msg = messages.front();
        messages.pop();
        return msg;
    }
};

// Broker class managing topics and partitions
class Broker {
private:
    std::map<std::string, std::vector<Partition>> topics;
    std::map<std::string, int> roundRobin; // For each topic, track next partition index
    std::mutex mtx;
public:
    Broker() {
        // Pre-configure topics and partitions.
        // For example, we create two topics: "topic1" (3 partitions) and "topic2" (2 partitions).
        topics["topic1"] = std::vector<Partition>(3);
        topics["topic2"] = std::vector<Partition>(2);
        roundRobin["topic1"] = 0;
        roundRobin["topic2"] = 0;
        std::cout << "Broker initialized with topics:\n";
        for (auto& t : topics) {
            std::cout << " - " << t.first << " with " << t.second.size() << " partitions.\n";
        }
    }

    bool topicExists(const std::string& topic) {
        std::lock_guard<std::mutex> lock(mtx);
        return topics.find(topic) != topics.end();
    }

    int getPartitionCount(const std::string& topic) {
        std::lock_guard<std::mutex> lock(mtx);
        if (topics.find(topic) != topics.end()) {
            return topics[topic].size();
        }
        return 0;
    }

    // Produce a message to a topic. Automatically assigns a partition (round-robin).
    int produceMessage(const std::string& topic, const Message& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        if (topics.find(topic) == topics.end()) {
            return -1; // topic not found
        }
        int partitionIndex = roundRobin[topic];
        topics[topic][partitionIndex].addMessage(msg);
        roundRobin[topic] = (partitionIndex + 1) % topics[topic].size();
        return partitionIndex;
    }

    // Consume a message from a specific partition (blocking call).
    Message consumeMessage(const std::string& topic, int partition) {
        // Get a reference to the partition outside the lock.
        Partition* partPtr = nullptr;
        {
            std::lock_guard<std::mutex> lock(mtx);
            partPtr = &topics[topic][partition];
        }
        return partPtr->getMessage();
    }
};

// Function to handle a client connection
void handleClient(int clientSocket, Broker& broker) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            std::cout << "Client disconnected or error occurred.\n";
            close(clientSocket);
            break;
        }
        std::string request(buffer);
        std::istringstream iss(request);
        std::string command;
        iss >> command;
        if (command == "PRODUCE") {
            std::string topic;
            int msgId;
            iss >> topic >> msgId;
            std::string msgContent;
            std::getline(iss, msgContent);
            if (!msgContent.empty() && msgContent[0] == ' ')
                msgContent.erase(0, 1);
            Message msg(msgId, msgContent);
            int partitionIndex = broker.produceMessage(topic, msg);
            if (partitionIndex < 0) {
                std::string error = "ERROR: Topic not found\n";
                write(clientSocket, error.c_str(), error.size());
            } else {
                std::string response = "Message produced to partition " + std::to_string(partitionIndex) + "\n";
                write(clientSocket, response.c_str(), response.size());
            }
        } else if (command == "CONSUME") {
            std::string topic;
            int partition;
            iss >> topic >> partition;
            if (!broker.topicExists(topic)) {
                std::string error = "ERROR: Topic not found\n";
                write(clientSocket, error.c_str(), error.size());
                continue;
            }
            if (partition < 0 || partition >= broker.getPartitionCount(topic)) {
                std::string error = "ERROR: Invalid partition index\n";
                write(clientSocket, error.c_str(), error.size());
                continue;
            }
            Message msg = broker.consumeMessage(topic, partition);
            std::string response = "Consumed: " + std::to_string(msg.id) + " " + msg.content + "\n";
            write(clientSocket, response.c_str(), response.size());
        } else {
            std::string error = "ERROR: Unknown command\n";
            write(clientSocket, error.c_str(), error.size());
        }
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(clientAddr);

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket.\n";
        return 1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket.\n";
        return 1;
    }
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Error listening on socket.\n";
        return 1;
    }

    std::cout << "Broker server started on port " << PORT << ".\n";
    Broker broker; // Broker with pre-configured topics and partitions

    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection.\n";
            continue;
        }
        std::cout << "New client connected.\n";
        std::thread clientThread(handleClient, clientSocket, std::ref(broker));
        clientThread.detach();
    }

    close(serverSocket);
    return 0;
}
