#include <iostream>
#include <csignal>      // Ctrl+C �ڵ鸵��
#include "GameServer.h"

GameServer server;
bool stopRequested = false;

// Ctrl+C �� ���� �ñ׳� ó��
void SignalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n[INFO] ���� ��û ������. ���� ���� ��...\n";
        stopRequested = true;
        server.Stop();
    }
}

int main() {
    const int port = 9999;

    // �ñ׳� �ڵ鷯 ���
    std::signal(SIGINT, SignalHandler);

    if (!server.Start(port)) {
        std::cerr << "[ERROR] ���� ���� ����\n";
        return 1;
    }

    std::cout << "[INFO] ������ ���� ���Դϴ�. Ctrl+C�� ������ �� �ֽ��ϴ�.\n";

    // ���� ������� ��ٸ��⸸ �� (���� �����尡 ���� ����)
    while (!stopRequested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "[INFO] ������ ���������� ����Ǿ����ϴ�.\n";
    return 0;
}
