#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

string username;
string password;

void newUser(SOCKET clientSocket, std::vector<char>& buffer) {
    //== ȸ������ ==
    //���̵�: newUser
    //��й�ȣ : 5678

    //[����] Register Success

    // send()/recv()

    cin.ignore(); // �ɼ� �������� ���� ��������
    cout << "���̵�: ";
    getline(std::cin, username);
    cout << "��й�ȣ: ";
    getline(std::cin, password);

    // [6] �α��� �޽��� ���� �� ����
    string message = "REGISTER:" + username + ":" + password + ":";
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] ���� ���� ���� (C++ ��Ÿ�Ϸ� ó��)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[����] " << response << std::endl;
    }
    else {
        cerr << "���� ���� ���� ����\n";
    }
}

bool clientLogin(SOCKET clientSocket, std::vector<char>& buffer)
{
    // send()/recv()

    cin.ignore(); // �ɼ� �������� ���� ��������
    cout << "���̵�: ";
    getline(std::cin, username);
    cout << "��й�ȣ: ";
    getline(std::cin, password);

    // [6] �α��� �޽��� ���� �� ����
    string message = "LOGIN:" + username + ":" + password + ":";
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] ���� ���� ���� (C++ ��Ÿ�Ϸ� ó��)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[����] " << response << std::endl;
        if (response == "Login Success") return true;
    }
    else {
        cerr << "���� ���� ���� ����\n";
    }
    return false;
}

void clientLogout(SOCKET clientSocket, std::vector<char>& buffer)
{
    // [6] �α׾ƿ� �޽��� ���� �� ����
    string message = "LOGOUT";
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] ���� ���� ���� (C++ ��Ÿ�Ϸ� ó��)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[����] " << response << std::endl;
    }
    else {
        cerr << "���� ���� ���� ����\n";
    }
}

void chat(SOCKET clientSocket, std::vector<char>& buffer)
{
    // send()/recv()
    string chat;
    
    cout << "['exit' ��ɾ� �Է½� ä�� ����]" << endl;
    cin.ignore(); // �ɼ� �������� ���� ��������
    while (true) { // exit ������ ���������� ä��
        cout << "ä�� �Է� : ";
        getline(std::cin, chat);

        if (chat == "exit") break; //strcmp - �ΰ� ���� ���ϰ� ������ 0�� ����

        // [6] �α��� �޽��� ���� �� ����
        string message = "CHAT:" + chat;
        send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

        // [7] ���� ���� ���� (C++ ��Ÿ�Ϸ� ó��)
        int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

        cout << bytesReceived;

        if (bytesReceived > 0) {
            string response(buffer.data(), bytesReceived);
            cout << "[����] " << response << std::endl;
        }
        else {
            cerr << "���� ���� ���� ����\n";
        }
    }
}

void recentChats(SOCKET clientSocket, std::vector<char>& buffer)
{
    cin.ignore(); // �ɼ� �������� ���� ��������

    // [6] �ֱ� 10�� ä�� �ҷ�����
    string message = "RECENTCHATS:" + username  ;
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] ���� ���� ���� (C++ ��Ÿ�Ϸ� ó��)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[����] " << endl << response << endl;
    }
    else {
        cerr << "���� ���� ���� ����\n";
    }
}

void activityLog(SOCKET clientSocket, std::vector<char>& buffer) {
    cin.ignore(); // �ɼ� �������� ���� ��������

    // [6] �ֱ� 10�� ä�� �ҷ�����
    string message = "ACTIVITY:" + username;
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] ���� ���� ���� (C++ ��Ÿ�Ϸ� ó��)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[����] " << endl << response << endl;
    }
    else {
        cerr << "���� ���� ���� ����\n";
    }
}

void showInitialMenu()
{
    cout << "=== �ʱ�ȭ�� ===" << endl;
    cout << "1. ȸ������" << endl
        << "2. �α���" << endl
        << "3. ����" << endl
        << "> ";
}

void showLoginMenu()
{
    cout << "=== �α��� �� �޴� ===" << endl;
    cout << "1. ä���ϱ�" << endl
        << "2. �ֱ� ��ȭ 10�� �ҷ�����" << endl
        << "3. Ȱ���α� ���" << endl
        << "4. �α׾ƿ�" << endl
        << "5. ����" << endl
        << "> ";
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    SOCKADDR_IN serverAddr;
    vector<char> buffer(1024);

    // Winsock �ʱ�ȭ
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // socket()
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // connect()
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    cout << "Connected to server." << endl;


    int choice;
    bool login = false;
    while (true) {
        if (!login){
            showInitialMenu();
            cin >> choice;
            switch (choice) {
                case 1:newUser(clientSocket, buffer); break;
                case 2: { 
                    login = clientLogin(clientSocket, buffer); // �α��� �Ǿ����� ���� login���� Ȯ��
                    break; }
                case 3:	cout << "Closing system" << endl; return 0;
                default: cout << "Wrong input" << endl; break;
            }
        }
        else {
            showLoginMenu();
            cin >> choice;
            switch (choice) {
            case 1:chat(clientSocket, buffer); break;
            case 2:recentChats(clientSocket, buffer); break;
            case 3:activityLog(clientSocket, buffer); break;
            case 4: {
                clientLogout(clientSocket, buffer);
                login = false;
                break;
            }
            case 5:	cout << "Closing system" << endl; return 0;
            default: cout << "Wrong input" << endl; break;
            }
        }
    }

    // [8] ����
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}