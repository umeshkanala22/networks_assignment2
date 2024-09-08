#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <nlohmann/json.hpp>

// Define constants
const int BUFFER_SIZE = 1024;

// Function to handle client requests
void handleClient(int clientSocket, const std::string& filename, int k, int p) {
    // Open file and read in chunks
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string chunk;
    int wordCount = 0;

    // Receive offset from client
    int offset;
    recv(clientSocket, &offset, sizeof(offset), 0);

    // Send words to client in chunks
    while (std::getline(file, chunk)) {
        std::istringstream wordStream(chunk);
        std::string word;
        int wordsSent = 0;
        while (wordStream >> word) {
            if (wordCount >= offset && wordsSent < k) {
                send(clientSocket, word.c_str(), word.length(), 0);
                send(clientSocket, "\n", 1, 0);
                wordCount++;
                wordsSent++;
            }
            if (wordsSent == k) break;
        }
        if (wordsSent == k) break;
    }

    // Send EOF to indicate end of file
    send(clientSocket, "EOF\n", 4, 0);
}

int main() {
    // Read config file
    nlohmann::json config;
    std::ifstream configFile("config.json");
    if (configFile.is_open()) {
        configFile >> config;
    } else {
        std::cerr << "Failed to open config file: config.json" << std::endl;
        return 1;
    }

    // Create socket and bind to port
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(config["server_port"].get<int>());
    if (!inet_aton("127.0.0.1", &serverAddress.sin_addr)) {
        std::cerr << "Failed to convert IP address" << std::endl;
        return 1;
    }

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    std::cout << "Server listening on port " << config["server_port"].get<int>() << std::endl;
    if (listen(serverSocket, 3) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }

    // Accept client connection
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0) {
        std::cerr << "Failed to accept client connection" << std::endl;
        return 1;
    }

    // Handle client requests
    handleClient(clientSocket, config["filename"].get<std::string>(), config["k"].get<int>(), config["p"].get<int>());
   std:: cout<<"handeling the client " << clientSocket << std::endl;

    // Close sockets
    // close(clientSocket);
    // close(serverSocket);

    return 0;
}
