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

const int BUFFER_SIZE = 2048;

void handleServer(int serverSocket, int p, int k) {
    string word;
    int wordCount = 0;
    map<string, int> wordFrequency;
    int total_wordsReceived = 0;
    int offset = 0;
    bool eofReached = false;

    while (!eofReached) {
        send(serverSocket, &offset, sizeof(offset), 0);
        cout << "sent request with offset " << offset << endl;

        int packetsReceived = 0;
        int limit = k / p + 1;
        if (k % p == 0) {
            limit = k / p;
        }

        while (packetsReceived < limit && !eofReached) {
            char buffer[BUFFER_SIZE];
            int bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0);
            buffer[bytesReceived] = '\0'; // Null-terminate the string

            if (bytesReceived == -1) {
                cerr << "Error receiving data from server" << endl;
                eofReached = true;
                break;
            }

            string response(buffer);
            if (response == "EOF\n") {
                cout << "Reached EOF" << endl;
                eofReached = true;
                break;
            }

            istringstream wordStream(response);

            int wordsReceived = 0;
            while (wordStream >> word) {
                wordFrequency[word]++;
                wordCount++;
                wordsReceived++;
                total_wordsReceived++;
                cout << "these are the current words received " << wordsReceived << endl;
                cout << "these are the total words received " << total_wordsReceived << endl;
            }
            packetsReceived++;
        }
        
        cout<<"out of the first while loop"<<endl;

        
        offset += k;
        if (eofReached) {
        cout << "Word Frequency:" << endl;
        for (auto& pair : wordFrequency) {
            cout << pair.first << ", " << pair.second << endl;
        }
            break;
        }
    }
    cout<<"retruning"<<endl;
    return;
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