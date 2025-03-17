#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <string>
#include <cstdlib>

// Message class to store message data
class Message {
public:
    int id;
    std::string content;
    int priority;
    std::chrono::system_clock::time_point timestamp;

    Message(int _id, std::string _content, int _priority)
        : id(_id), content(_content), priority(_priority), timestamp(std::chrono::system_clock::now()) {}
    
    // For comparison in priority queue (higher priority gets processed first)
    bool operator<(const Message& other) const {
        return priority < other.priority; // Reverse the operator to make priority queue max-heap
    }
};

// MessageQueue class that provides thread-safe message queuing
class MessageQueue {
private:
    std::priority_queue<Message> messageQueue;
    std::mutex mtx;
    std::condition_variable cv;
    
public:
    // Method to send message (thread-safe)
    void sendMessage(const Message& msg) {
        std::unique_lock<std::mutex> lock(mtx);
        messageQueue.push(msg);
        cv.notify_one(); // Notify one waiting consumer
    }

    // Method to receive message (thread-safe)
    Message receiveMessage() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !messageQueue.empty(); }); // Wait until the queue is not empty
        Message msg = messageQueue.top();
        messageQueue.pop();
        return msg;
    }
};

// Producer class to simulate message sending
class Producer {
private:
    MessageQueue& msgQueue;
    int id;
    
public:
    Producer(MessageQueue& mq, int producerId) : msgQueue(mq), id(producerId) {}

    void produce(int numMessages) {
        for (int i = 1; i <= numMessages; ++i) {
            int priority = rand() % 10 + 1;  // Generate random priority (1 to 10)
            std::string content = "Message from Producer " + std::to_string(id);
            Message msg(i, content, priority);
            msgQueue.sendMessage(msg);
            std::cout << "Produced Message ID: " << msg.id << ", Content: " << msg.content 
                      << ", Priority: " << msg.priority << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work delay
        }
    }
};

// Consumer class to simulate message receiving
class Consumer {
private:
    MessageQueue& msgQueue;
    int id;

public:
    Consumer(MessageQueue& mq, int consumerId) : msgQueue(mq), id(consumerId) {}

    void consume() {
        while (true) {
            Message msg = msgQueue.receiveMessage();
            std::cout << "Consumer " << id << " consumed Message ID: " << msg.id 
                      << ", Content: " << msg.content << ", Priority: " << msg.priority << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Simulate processing delay
        }
    }
};

// Main function
int main(int argc, char* argv[]) {
    // Check for command line arguments: number of producers and consumers
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <num_producers> <num_consumers>\n";
        return 1;
    }

    int numProducers = std::atoi(argv[1]);
    int numConsumers = std::atoi(argv[2]);

    if (numProducers <= 0 || numConsumers <= 0) {
        std::cerr << "Number of producers and consumers must be positive integers.\n";
        return 1;
    }

    MessageQueue messageQueue;

    // Create producer and consumer threads
    std::vector<std::thread> producerThreads;
    std::vector<std::thread> consumerThreads;

    // Start producer threads
    for (int i = 0; i < numProducers; ++i) {
        producerThreads.push_back(std::thread([&messageQueue, i]() {
            Producer producer(messageQueue, i + 1);
            producer.produce(10); // Each producer will produce 10 messages
        }));
    }

    // Start consumer threads
    for (int i = 0; i < numConsumers; ++i) {
        consumerThreads.push_back(std::thread([&messageQueue, i]() {
            Consumer consumer(messageQueue, i + 1);
            consumer.consume(); // Consumers will consume messages continuously
        }));
    }

    // Join producer threads (wait for them to finish)
    for (auto& th : producerThreads) {
        th.join();
    }

    // Consumers will run indefinitely, so detach them
    for (auto& th : consumerThreads) {
        th.detach();
    }

    std::cout << "All producer threads finished. Consumers are still running...\n";

    return 0;
}
