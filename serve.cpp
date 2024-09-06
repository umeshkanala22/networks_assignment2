#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <json/json.h>

// Define constants
const int BUFFER_SIZE = 1024;

// Function to handle client requests
void handleClient(int clientSocket, std::string filename, int k, int p) {
    // Open file and read in chunks
    std::ifstream file(filename);
    std::string fileContents;
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
    Json::Value config;
    std::ifstream configFile("config.json");
    configFile >> config;

    // Create socket and bind to port
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(config["server_port"].asInt());
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Listen for incoming connections
    listen(serverSocket, 3);

    // Accept client connection
    int clientSocket = accept(serverSocket, NULL, NULL);

    // Handle client requests
    handleClient(clientSocket, config["filename"].asString(), config["k"].asInt(), config["p"].asInt());

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}