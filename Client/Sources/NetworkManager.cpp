#include "NetworkManager.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

bool NetworkManager::Connect(const std::string& ip, int port) {
    // Winsock �ʱ�ȭ
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[����] WSAStartup ����\n";
        return false;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[����] ���� ���� ����\n";
        return false;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());  // ����õ�� ��������� �ϴ� ���

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[����] ���� ���� ����\n";
        closesocket(sock);
        return false;
    }

    std::cout << "[����] ������ �����\n";
    return true;
}

void NetworkManager::StartRecvThread() {
    running = true;
    recvThread = std::thread(&NetworkManager::RecvLoop, this);
}

void NetworkManager::RecvLoop() {
    char buffer[512];
    while (running) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "[���� ���� ���� or ����]\n";
            running = false;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "[����] " << buffer << std::endl;
    }
}

void NetworkManager::Send(const std::string& msg) {
    if (sock != INVALID_SOCKET) {
        send(sock, msg.c_str(), static_cast<int>(msg.length()), 0);
    }
}

void NetworkManager::Stop() {
    running = false;

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    if (recvThread.joinable()) {
        recvThread.join();
    }

    WSACleanup();
}
