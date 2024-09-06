#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

// Define constants
const int BUFFER_SIZE = 1024;

// Function to handle server responses
void handleServer(int serverSocket, int p) {
    // Receive words from server
    std::string word;
    int wordCount = 0;
    std::map<std::string, int> wordFrequency;
    while (true) {
        char buffer[BUFFER_SIZE];
        recv(serverSocket, buffer, BUFFER_SIZE, 0);
        std::string response(buffer);
        if (response == "EOF\n") break;
        std::istringstream wordStream(response);
        int wordsReceived = 0;
        while (wordStream >> word) {
            wordFrequency[word]++;
            wordCount++;
            wordsReceived++;
            if (wordsReceived == p) break;
        }
    }

    // Print word frequency
    for (auto& pair : wordFrequency) {
        std::cout << pair.first << ", " << pair.second << std::endl;
    }
}

int main() {
    // Read config file
    nlohmann::json config;
    std::ifstream configFile("config.json");
    configFile >> config;

    // Create socket and connect to server
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(config["server_port"].get<int>());
    inet_pton(AF_INET, config["server_ip"].get<std::string>().c_str(), &serverAddress.sin_addr);
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Send offset to server
    int offset = 0;
    send(clientSocket, &offset, sizeof(offset), 0);

    // Handle server responses
    handleServer(clientSocket, config["p"].get<int>());

    // Close socket
    close(clientSocket);

    return 0;
}