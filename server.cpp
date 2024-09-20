#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <nlohmann/json.hpp>

using namespace std;

const int BUFFER_SIZE = 4096;

void handleClient(int clientSocket, const string& filename, int k, int p) { 
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    vector<string> words;
    string chunk;
    while (getline(file, chunk)) {
        istringstream wordStream(chunk);
        string word;
        while (wordStream >> word) {
            words.push_back(word);
        }
    }
    cout<<words.size()<<endl;

    int offset;
    int wordCount = 0;
    bool eof = false;
    while (!eof && wordCount<words.size()) {
        recv(clientSocket, &offset, sizeof(offset), 0);
        wordCount=offset;
        cout << "wordCount: " << wordCount << endl;

        int wordsSent = 0;
        vector<string> packet;
        int packetWords = 0;
        while (wordsSent < k && wordCount < words.size()) {
            packet.push_back(words[wordCount]);
            wordsSent++;
            wordCount++;
            packetWords++;
            if (packetWords == p) {
                packetWords = 0;
                string packetData="";
                packetData=packet[0];
                for(int i=1;i<packet.size();i++){
                    packetData+=",";
                    packetData+=packet[i];
                }

                if(wordCount == words.size()) {
                    packetData+=",EOF\n";
                    send(clientSocket, packetData.c_str(), packetData.length(), 0);
                    eof = true;
                    break;

                }
                packetData+=",\n";
            
                send(clientSocket, packetData.c_str(), packetData.length(), 0);
                packet.clear();
            }
        }
        if (wordCount ==words.size()){
            eof=true;

        }
        cout<<"out of first while loop"<<endl;
        if (packetWords != 0) {
            string packetData;
            for (const auto& w : packet) {
                packetData +=","+w;
            }
            packetData+=",EOF\n";
            send(clientSocket, packetData.c_str(), packetData.length(), 0);
            packet.clear();
        }
        
        if (wordCount >= words.size()-1) {

            break;
        }
    }
    cout<<wordCount<<endl;
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
    std::cout << "Handling client " << clientSocket << std::endl;

    // Close sockets
    close(serverSocket);
    close(clientSocket);


    return 0;
}