#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

using namespace std;

// Function to initialize WinSock
bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients) {
    //send and receive messages between clients to free the server
    cout << "client connected" << endl;

    char buffer[4096];

    while (1) {
        int byterecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (byterecvd <= 0) {
            cout << "client disconnected" << endl;
            break;
        }
        string message(buffer, byterecvd);
        cout << "message from client: " << message << endl;

        for (auto client : clients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.length(), 0); // message sent to every client except the sender
            }
        }
    }

    auto it = find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }

    closesocket(clientSocket);
}

int main() {
    if (!Initialize()) {
        cout << "WinSock initialization failed" << endl;
        return 1;
    }

    cout << "Server program" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    // Create address structure
    int port = 12345;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert the IP address (0.0.0.0) and put it inside sin_addr in binary format
    if (InetPton(AF_INET, _T("0.0.0.0"), &serverAddr.sin_addr) != 1) {
        cout << "Setting address structure failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Bind code
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    cout << "listening to port :" << port << endl;
    vector<SOCKET> clients;

    while (1) {
        // Accept connection from the client
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "invalid client socket" << endl;
            continue; // skip the rest of the loop and wait for the next connection
        }

        clients.push_back(clientSocket);
        thread t1(InteractWithClient, clientSocket, std::ref(clients));
        t1.detach();
    }

    closesocket(listenSocket);

    // Cleanup
    WSACleanup();

    return 0;
}
