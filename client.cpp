#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
using namespace std;

const int BUFFER_SIZE = 2048;

// void handleServer(int serverSocket, int p) {
//     cout<<"enterd the client function"<<endl;
//     string word;
//     int wordCount = 0;
//     map<std::string, int> wordFrequency;
//     bool bk=false;
//     while (true) {
//         char buffer[BUFFER_SIZE];
        
//         recv(serverSocket, buffer, BUFFER_SIZE, 0);
//         string response(buffer);
//         if (response == "EOF\n") break;
//         istringstream wordStream(response);
//         int wordsReceived = 0;
//         while (wordStream >> word) {
//             wordFrequency[word]++;
//             wordCount++;
//             wordsReceived++;
//             if (wordsReceived == p){ 
//                 bk=true;
//                 break;}
//         }
//         if(bk==true){
//             break;
//         }
//     }
//     cout<<"I have reached on something"<<endl;
//     for (auto& pair : wordFrequency) {
//         cout << pair.first << ", " << pair.second <<endl;
//     }
// }
void handleServer(int serverSocket, int p,int k) {
    cout << "Entered the client function" << endl;
    string word;
    int wordCount = 0;
    map<std::string, int> wordFrequency;
    int total_wordsReceived = 0;
    while (true) {
        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == -1) {
            cerr << "Error receiving data from server" << endl;
            break;
        }
        string response(buffer);
        if (response == "EOF\n") break;
        istringstream wordStream(response);

        int wordsReceived = 0;
        
        while (wordStream >> word) {
            wordFrequency[word]++;
            wordCount++;
            wordsReceived++;
            total_wordsReceived++;
            cout<<"these are the current words received "<<wordsReceived<<endl;
            cout<<"theses are the total words recevied "<<total_wordsReceived<<endl;
            if (wordsReceived == p){
                cout<<"I am broke"<<endl;
                break;
            }
            cout<<"in the while loop 1"<<endl;
        }
        if (total_wordsReceived == k) break;

    }

    cout << "I have reached something" << endl;
    for (auto& pair : wordFrequency) {
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
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

   
    int offset = 0;
    cout<<"going to send request"<<endl;
    send(clientSocket, &offset, sizeof(offset), 0);
    cout<<"sending request"<<endl;
    
    handleServer(clientSocket, config["p"].get<int>(),config["k"].get<int>());
    cout<<"done"<<endl;

    // Close socket
    close(clientSocket);
    
   

    return 0;
}