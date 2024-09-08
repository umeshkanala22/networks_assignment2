#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<vector>
#include <nlohmann/json.hpp>
using namespace std;

const int BUFFER_SIZE = 2048;

void handleClient(int clientSocket, const std::string& filename, int k, int p) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    string chunk;
    int wordCount = 0;
    int packetCount = 0;

    int offset;
    recv(clientSocket, &offset, sizeof(offset), 0);

    vector<string> packet;
    while (getline(file, chunk)) {
        istringstream wordStream(chunk);
        string word;
        int wordsSent = 0;
        while (wordStream >> word) {
            if (wordCount >= offset && wordsSent < k) {
                packet.push_back(word);
                wordCount++;
                wordsSent++;
                if (packet.size() == p) {
                    // Send packet
                    string packetData;
                    for (const auto& w : packet) {
                        packetData += w + "\n";
                    }
                    send(clientSocket, packetData.c_str(), packetData.length(), 0);
                    packet.clear();
                    packetCount++;
                }
            }
            if (wordsSent == k) break;
        }
        if (wordsSent == k) break;
    }

    if (!packet.empty()) {
        string packetData;
        for (const auto& w : packet) {
            packetData += w + "\n";
        }
        send(clientSocket, packetData.c_str(), packetData.length(), 0);
    }

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
    close(clientSocket);
    close(serverSocket);

    return 0;
}
