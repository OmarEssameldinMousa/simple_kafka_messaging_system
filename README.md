# Kafka-like Messaging System

## Overview
This project is a simplified, Kafka-inspired messaging system implemented in C++ using a client–server architecture. It consists of three main components:

- **Broker Server**: Pre-configures topics and partitions, handles TCP connections from producers and consumers concurrently, and manages message flow.
- **Producer Client**: Connects to the broker and lets you choose a topic to produce messages. It supports two modes:
  - **Single Message Mode**: The producer sends one message as specified by the user.
  - **Stream Mode**: The producer sends a continuous stream of log messages at one-second intervals.
- **Consumer Client**: Connects to the broker and subscribes to an available topic (and optionally a specific partition) to consume messages.

## Features
- **Pre-configured Topics and Partitions**: The broker internally sets up topics (e.g., "topic1" and "topic2") with a fixed number of partitions.
- **Round-Robin Message Assignment**: Messages sent by producers are automatically distributed among partitions in a round-robin manner.
- **Simple TCP-based Communication**: Utilizes sockets for network communication between clients and the broker.
- **Thread-Safe Message Handling**: Uses mutexes and condition variables to ensure safe concurrent access to shared data structures.
- **Dual Mode Production**: Producers can send either single messages or a continuous stream of log messages.

## Compilation

Before running the project, ensure that all files are compiled. Use the following commands to compile the broker, producer, and consumer:

```bash
g++ broker_server.cpp -o broker_server -pthread
g++ producer_client.cpp -o producer_client -pthread
g++ consumer_client.cpp -o consumer_client -pthread
```

## Usage

### 1. Start the Broker Server
Run the broker server first to initialize topics and partitions. It listens on port 8080.

```bash
./broker_server
```

### 2. Start a Producer Client
The producer sends messages to a selected topic. It supports two modes: sending a single message or continuously streaming log messages every second.

#### Single Message Mode:
```bash
./producer_client <topic_name> single
```
Example:
```bash
./producer_client topic1 single
```
The producer will prompt you to enter a message.

#### Stream Mode (Log Stream):
```bash
./producer_client <topic_name> stream
```
Example:
```bash
./producer_client topic1 stream
```
In stream mode, the producer sends log messages continuously at one-second intervals.

### 3. Start a Consumer Client
The consumer client subscribes to a specific topic and consumes messages. Optionally, it can subscribe to a specific partition within that topic.

```bash
./consumer_client <topic_name>
```
Example:
```bash
./consumer_client topic1
```

To consume messages from a specific partition:
```bash
./consumer_client <topic_name> <partition_number>
```
Example:
```bash
./consumer_client topic1 1
```

### Cleanup

After running the project, you can clean up any generated binaries with the following command:

```bash
rm -f broker_server producer_client consumer_client
```

## Advantages
- **Clarity and Simplicity**:  
  The codebase clearly demonstrates the core concepts of a messaging system—topics, partitions, producers, and consumers—making it an excellent learning tool.
  
- **Lightweight and Modular**:  
  Each component (broker, producer, consumer) is implemented in separate files, which simplifies testing and future enhancements.
  
- **Multithreading and Synchronization**:  
  The project makes use of modern C++ multithreading techniques to handle concurrent connections and ensure thread safety.
  
- **Flexible Production Modes**:  
  Producers can easily switch between sending a single message and streaming log messages, showcasing dynamic behavior.

## Disadvantages
- **Ephemeral Data Storage**:  
  Consumed messages are not stored permanently. Once a message is consumed from a partition, it is removed from the broker. This means data is transient and cannot be retrieved later.
  
- **No Persistence**:  
  There is no mechanism to persist messages on disk. In the event of a broker crash or restart, all in-memory data is lost.
  
- **Limited Scalability and Fault Tolerance**:  
  Without features like consumer groups, offset tracking, or replication, the system is not designed for high throughput or robust fault tolerance as seen in production-grade systems like Apache Kafka.
  
- **Basic Error Handling**:  
  While basic error checking is implemented, the system could be further improved to handle network errors, malformed requests, and other runtime issues more gracefully.

## Future Developments
- **Message Persistence and Retention**:  
  Introduce a disk-based storage system to retain messages, similar to Kafka's commit log and retention policies.
  
- **Consumer Group and Offset Management**:  
  Implement support for consumer groups and offset tracking, allowing multiple consumers to share the load and resume consumption where they left off.
  
- **Replication and Fault Tolerance**:  
  Develop mechanisms for message replication across multiple broker instances to ensure data durability and high availability.
  
- **Dynamic Topic Configuration**:  
  Enhance the broker to allow dynamic creation, deletion, and configuration of topics and partitions through an API or command-line interface.
  
- **Enhanced Protocol and Security**:  
  Improve the communication protocol for better performance and introduce security features (e.g., encryption, authentication) for production environments.
  
- **Monitoring and Management Tools**:  
  Build a dashboard or command-line monitoring tools to visualize message flow, track system performance, and manage broker health.

## Conclusion
This Kafka-like messaging system provides a solid foundation for understanding distributed messaging concepts and client–server architecture. While it demonstrates key features such as topics, partitions, and real-time messaging, further enhancements—especially in persistence, scalability, and fault tolerance—are necessary to evolve it into a production-ready solution.
