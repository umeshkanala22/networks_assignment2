#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <map>

using namespace std;

const int BUFFER_SIZE = 4096;

void handleServer(int serverSocket, int p, int k) {
    int offset = 0;
    map<string, int> wordFrequency;
    bool eofReached = false;

    while (!eofReached) {
        send(serverSocket, &offset, sizeof(offset), 0);
        int numberofwords = 0;

        while (numberofwords < k && !eofReached) {
            char buffer[BUFFER_SIZE];
            int bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0);
            buffer[bytesReceived] = '\0';

            // Remove newline character from packet
            string packet(buffer);
            packet.erase(remove(packet.begin(), packet.end(), '\n'), packet.end());

            istringstream packetStream(packet);
            string word;
            while (getline(packetStream, word, ',')) {
                // Check for empty words
                if (word.empty()) {
                    continue;
                }

                string temp = "EOF";
                if (word == temp) {
                    eofReached = true;
                    cout << "EOF reached" << endl;
                    break;
                }

                cout << "this is the word " << word << " completed" << endl;

                wordFrequency[word]++;
                numberofwords++;
            }
        }
        offset += k;
    }

    for (const auto& pair : wordFrequency) {
        cout << pair.first << ", " << pair.second << endl;
    }
}

int main() {
    nlohmann::json config;
    std::ifstream configFile("config.json");
    configFile >> config;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(config["server_port"].get<int>());
    inet_pton(AF_INET, config["server_ip"].get<std::string>().c_str(), &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Failed to connect to server" << endl;
        return 1;
    }

    handleServer(clientSocket, config["p"].get<int>(), config["k"].get<int>());
    cout << "done" << endl;

    // Close socket
    close(clientSocket);

    return 0;
}