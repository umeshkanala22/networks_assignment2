// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <string>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <nlohmann/json.hpp>
// #include <map>
// #include <thread>

// using namespace std;

// const int BUFFER_SIZE = 4096;

// void handleServer(int serverSocket, int p, int k) {
//     int offset = 0;
//     map<string, int> wordFrequency;
//     bool eofReached = false;

//     while (!eofReached) {
//         send(serverSocket, &offset, sizeof(offset), 0);
//         int numberofwords = 0;

//         while (numberofwords < k && !eofReached) {
//             char buffer[BUFFER_SIZE];
//             int bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0);
//             buffer[bytesReceived] = '\0';

//             // Remove newline character from packet
//             string packet(buffer);
//             packet.erase(remove(packet.begin(), packet.end(), '\n'), packet.end());

//             istringstream packetStream(packet);
//             string word;
//             while (getline(packetStream, word, ',')) {
//                 // Check for empty words
//                 if (word.empty()) {
//                     continue;
//                 }

//                 string temp = "EOF";
//                 if (word == temp) {
//                     eofReached = true;
//                     cout << "EOF reached" << endl;
//                     break;
//                 }

//                 cout << "this is the word " << word << " completed" << endl;

//                 wordFrequency[word]++;
//                 numberofwords++;
//             }
//         }
//         offset += k;
//     }

//     for (const auto& pair : wordFrequency) {
//         cout << pair.first << ", " << pair.second << endl;
//     }
// }

// void addClient(int client_id){
//     nlohmann::json config;
//     std::ifstream configFile("config.json");
//     configFile >> config;

//     int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if(clientSocket < 0){
//         std::cerr << "Client " << client_id << ": Failed to create socket" << std::endl;
//         return;
//     }

//     sockaddr_in serverAddress;
//     serverAddress.sin_family = AF_INET;
//     serverAddress.sin_port = htons(config["server_port"].get<int>());
//     inet_pton(AF_INET, config["server_ip"].get<std::string>().c_str(), &serverAddress.sin_addr);

//     if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
//         cerr << "Failed to connect to server" << endl;
//         return;
//     }

//     handleServer(clientSocket, config["p"].get<int>(), config["k"].get<int>());
//     cout << "Client " << client_id << ": done" << endl;

//     // Close socket
//     close(clientSocket);
// }

// int main() {
//     int num_clients[] = {1, 5, 9, 13, 17, 21, 25, 29};

//     for(int i=0;i<8;i++){
//         int clients = num_clients[i];
//         vector<thread> client_threads;

//         for(int j=0;j<clients;j++){
//             client_threads.emplace_back(addClient, j+1);
//         }

//         // Waiting for all threads to finish
//         for(auto&t:client_threads){
//             t.join();
//         }

//         cout<<"Test completed for "<<clients<<" concurrent clients."<<endl;
//     }

//     return 0;
// }
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
#include <pthread.h>

using namespace std;

const int BUFFER_SIZE = 4096;

void* handleServer(void* arg) {
    int* params = (int*)arg;
    int serverSocket = params[0];
    int p = params[1];
    int k = params[2];

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

    return NULL;
}

void* addClient(void* arg) {
    int* params = (int*)arg;
    int client_id = params[0];

    nlohmann::json config;
    std::ifstream configFile("config.json");
    configFile >> config;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        std::cerr << "Client " << client_id << ": Failed to create socket" << std::endl;
        return NULL;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(config["server_port"].get<int>());
    inet_pton(AF_INET, config["server_ip"].get<std::string>().c_str(), &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Failed to connect to server" << endl;
        return NULL;
    }

    int paramsForHandleServer[3] = {clientSocket, config["p"].get<int>(), config["k"].get<int>()};
    pthread_t serverThread;
    pthread_create(&serverThread, NULL, handleServer, paramsForHandleServer);
    pthread_join(serverThread, NULL);

    cout << "Client " << client_id << ": done" << endl;

    // Close socket
    close(clientSocket);
    return NULL;
}

int main() {
    int num_clients[] = {1, 5, 9, 13, 17, 21, 25, 29};
    pthread_t* client_threads;

    for(int i=0;i<8;i++){
        int clients = num_clients[i];
        client_threads = new pthread_t[clients];

        for(int j=0;j<clients;j++){
            int* params = new int[1];
            params[0] = j+1;
            pthread_create(&client_threads[j], NULL, addClient, params);
        }

        // Waiting for all threads to finish
        for(int j=0;j<clients;j++){
            pthread_join(client_threads[j], NULL);
        }

        delete[] client_threads;

        cout<<"Test completed for "<<clients<<" concurrent clients."<<endl;
    }

    return 0;
}