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
    //== 회원가입 ==
    //아이디: newUser
    //비밀번호 : 5678

    //[서버] Register Success

    // send()/recv()

    cin.ignore(); // 옵션 선택으로 인한 버퍼정리
    cout << "아이디: ";
    getline(std::cin, username);
    cout << "비밀번호: ";
    getline(std::cin, password);

    // [6] 로그인 메시지 구성 및 전송
    string message = "REGISTER:" + username + ":" + password + ":";
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] 서버 응답 수신 (C++ 스타일로 처리)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[서버] " << response << std::endl;
    }
    else {
        cerr << "서버 응답 수신 실패\n";
    }
}

bool clientLogin(SOCKET clientSocket, std::vector<char>& buffer)
{
    // send()/recv()

    cin.ignore(); // 옵션 선택으로 인한 버퍼정리
    cout << "아이디: ";
    getline(std::cin, username);
    cout << "비밀번호: ";
    getline(std::cin, password);

    // [6] 로그인 메시지 구성 및 전송
    string message = "LOGIN:" + username + ":" + password + ":";
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] 서버 응답 수신 (C++ 스타일로 처리)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[서버] " << response << std::endl;
        if (response == "Login Success") return true;
    }
    else {
        cerr << "서버 응답 수신 실패\n";
    }
    return false;
}

void clientLogout(SOCKET clientSocket, std::vector<char>& buffer)
{
    // [6] 로그아웃 메시지 구성 및 전송
    string message = "LOGOUT";
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] 서버 응답 수신 (C++ 스타일로 처리)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[서버] " << response << std::endl;
    }
    else {
        cerr << "서버 응답 수신 실패\n";
    }
}

void chat(SOCKET clientSocket, std::vector<char>& buffer)
{
    // send()/recv()
    string chat;
    
    cout << "['exit' 명령어 입력시 채팅 종료]" << endl;
    cin.ignore(); // 옵션 선택으로 인한 버퍼정리
    while (true) { // exit 전까지 지속적으로 채팅
        cout << "채팅 입력 : ";
        getline(std::cin, chat);

        if (chat == "exit") break; //strcmp - 두개 값을 비교하고 같으면 0이 나옴

        // [6] 로그인 메시지 구성 및 전송
        string message = "CHAT:" + chat;
        send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

        // [7] 서버 응답 수신 (C++ 스타일로 처리)
        int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

        cout << bytesReceived;

        if (bytesReceived > 0) {
            string response(buffer.data(), bytesReceived);
            cout << "[서버] " << response << std::endl;
        }
        else {
            cerr << "서버 응답 수신 실패\n";
        }
    }
}

void recentChats(SOCKET clientSocket, std::vector<char>& buffer)
{
    cin.ignore(); // 옵션 선택으로 인한 버퍼정리

    // [6] 최근 10개 채팅 불러오기
    string message = "RECENTCHATS:" + username  ;
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] 서버 응답 수신 (C++ 스타일로 처리)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[서버] " << endl << response << endl;
    }
    else {
        cerr << "서버 응답 수신 실패\n";
    }
}

void activityLog(SOCKET clientSocket, std::vector<char>& buffer) {
    cin.ignore(); // 옵션 선택으로 인한 버퍼정리

    // [6] 최근 10개 채팅 불러오기
    string message = "ACTIVITY:" + username;
    send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // [7] 서버 응답 수신 (C++ 스타일로 처리)
    int bytesReceived = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

    cout << bytesReceived;

    if (bytesReceived > 0) {
        string response(buffer.data(), bytesReceived);
        cout << "[서버] " << endl << response << endl;
    }
    else {
        cerr << "서버 응답 수신 실패\n";
    }
}

void showInitialMenu()
{
    cout << "=== 초기화면 ===" << endl;
    cout << "1. 회원가입" << endl
        << "2. 로그인" << endl
        << "3. 종료" << endl
        << "> ";
}

void showLoginMenu()
{
    cout << "=== 로그인 후 메뉴 ===" << endl;
    cout << "1. 채팅하기" << endl
        << "2. 최근 대화 10개 불러오기" << endl
        << "3. 활동로그 통계" << endl
        << "4. 로그아웃" << endl
        << "5. 종료" << endl
        << "> ";
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    SOCKADDR_IN serverAddr;
    vector<char> buffer(1024);

    // Winsock 초기화
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
                    login = clientLogin(clientSocket, buffer); // 로그인 되었는지 여부 login으로 확인
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

    // [8] 정리
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}