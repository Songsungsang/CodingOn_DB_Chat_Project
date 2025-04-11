#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mysql/jdbc.h>

#pragma comment(lib, "ws2_32.lib")

const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "root";
std::string DB_PASS = "";
std::string DB_NAME = "";
std::unique_ptr<sql::Connection> conn;

bool serverConnect() { // ������ MySQL ùȸ ����
    std::cout << "Password: ";
    getline(std::cin, DB_PASS);
    std::cout << "DB Name: ";
    getline(std::cin, DB_NAME);
    try {

        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        conn = std::unique_ptr<sql::Connection>(
            driver->connect(DB_HOST, DB_USER, DB_PASS)
        );
        conn->setSchema(DB_NAME);

        std::cout << "[DB Connected]" << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL DB Connect Error: " << e.what() << std::endl;
        return false;
    }
}

void handleRegister(const std::string& msg, SOCKET clientSocket) { // ȸ������ ó��
    size_t pos1 = msg.find(":", 9); // REGISTER:
    size_t pos2 = msg.find(":", pos1 + 1);

    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        std::string error = "Invalid Format";
        send(clientSocket, error.c_str(), error.length(), 0);
        closesocket(clientSocket);
        return;
    }

    std::string username = msg.substr(9, pos1 - 9);
    std::string password = msg.substr(pos1 + 1, pos2 - pos1 - 1);

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("INSERT INTO users (username, password) VALUES(?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, password);

        int num = pstmt->executeUpdate();

        std::string result = (num == 1 ? "Register Success" : "Register Failed");
        send(clientSocket, result.c_str(), result.length(), 0);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::string err = "Register Failed";
        send(clientSocket, err.c_str(), err.length(), 0);
    }
}

void handleLogin(const std::string& msg, SOCKET clientSocket) { // �α��� ó��
    std::string result;

    size_t pos1 = msg.find(":", 6); // LOGIN:
    size_t pos2 = msg.find(":", pos1 + 1);

    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        std::string error = "Invalid Format";
        send(clientSocket, error.c_str(), error.length(), 0);
        closesocket(clientSocket);
        return;
    }

    std::string username = msg.substr(6, pos1 - 6);
    std::string password = msg.substr(pos1 + 1, pos2 - pos1 - 1);

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt( // �α��� ���� ���� Ȯ��
            conn->prepareStatement("SELECT * FROM users WHERE username = ? AND password = ?")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, password);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            result = "Login Success"; // �α��� ���� �޼��� ����

            int user_id = res->getInt("user_id");

            sockaddr_in clientAddr;
            int addrSize = sizeof(clientAddr);
            getpeername(clientSocket, (sockaddr*)&clientAddr, &addrSize); // ������ ���� ��ȸ

            char ip_address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), ip_address, INET_ADDRSTRLEN); // ip_address�� �� �Է�

            std::unique_ptr<sql::PreparedStatement> insertQuery( // �α��� ��� �����
                conn->prepareStatement("INSERT INTO user_sessions (user_id, ip_address) VALUES(?, ?)")
            );
            insertQuery->setInt(1, user_id);
            insertQuery->setString(2, ip_address);

            int n = insertQuery->executeUpdate(); // ����� ���� ���ڰ� ��ȯ
            std::cout << n << " session recorded" << std::endl;
        }
        else {
            result = "Login Failed";
        }
        send(clientSocket, result.c_str(), result.length(), 0);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::string err = "DB Error";
        send(clientSocket, err.c_str(), err.length(), 0);
    }
}

void handleLogout(const std::string& msg, SOCKET clientSocket) { // �α׾ƿ� ó��
    try {
        sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);
        getpeername(clientSocket, (sockaddr*)&clientAddr, &addrSize); // ������ ���� ��ȸ

        char ip_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ip_address, INET_ADDRSTRLEN); // ip_address�� �� �Է�

        std::unique_ptr<sql::PreparedStatement> selectQuery( // ip �ּ� �������� �α��� �ð� ã��
            conn->prepareStatement("SELECT login_time FROM user_sessions WHERE ip_address = ? ORDER BY session_id DESC LIMIT 1")
        );
        selectQuery->setString(1, ip_address);

        std::unique_ptr<sql::ResultSet> res(selectQuery->executeQuery());
        if (res->next()) { // ����� �ִٸ�
            std::string login_time = res->getString("login_time"); // �α��� �ð� �޾ƿ���

            std::unique_ptr<sql::PreparedStatement> pstmt( // ip �ּҿ� ���� �ֱ� �α��� �ð� �������� �α׾ƿ� �ð� ������Ʈ
                conn->prepareStatement("Update user_sessions SET logout_time = CURRENT_TIMESTAMP() WHERE "
                    "ip_address = ? AND login_time = ?")
            );
            pstmt->setString(1, ip_address);
            pstmt->setString(2, login_time);

            int n = pstmt->executeUpdate(); // ����� ���� ���ڰ� ��ȯ
            std::cout << n << " session updated" << std::endl;

            std::string result = (n == 1 ? "Logout Success" : "Logout Failed");
            send(clientSocket, result.c_str(), result.length(), 0);
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::string err = "DB Error";
        send(clientSocket, err.c_str(), err.length(), 0);
    }
}

void handleChat(const std::string& msg, SOCKET clientSocket) { // ������ ä�� ó��

    if (msg.length() <= 5) { // �ּ� �޼��� ���̵� ���� ���Ѵٸ�
        std::string error = "Invalid Format";
        send(clientSocket, error.c_str(), error.length(), 0);
        closesocket(clientSocket);
        return;
    }

    std::string content = msg.substr(5); // ä�� ����

    try {
        sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);
        getpeername(clientSocket, (sockaddr*)&clientAddr, &addrSize); // ������ ���� ��ȸ

        char ip_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ip_address, INET_ADDRSTRLEN); // ip_address�� �� �Է�

        std::unique_ptr<sql::PreparedStatement> selectQuery( // ip �ּ� �������� ���� id ã��
            conn->prepareStatement("SELECT user_id FROM user_sessions WHERE ip_address = ? ORDER BY session_id DESC LIMIT 1")
        );
        selectQuery->setString(1, ip_address);

        std::unique_ptr<sql::ResultSet> res(selectQuery->executeQuery());
        if (res->next()) { // ����� �ִٸ�
            std::string user_id = res->getString("user_id"); // ����� id �޾ƿ���

            std::unique_ptr<sql::PreparedStatement> pstmt( // ä�� �α� ���
                conn->prepareStatement("INSERT INTO message_log (sender_id, content) VALUES(?, ?)")
            );
            pstmt->setString(1, user_id);
            pstmt->setString(2, content);

            int n = pstmt->executeUpdate(); // ����� ���� ���ڰ� ��ȯ
            std::cout << n << " chat logged" << std::endl;

            std::string result = content; // ä�� �Է� �ٽ� ��ȯ
            send(clientSocket, result.c_str(), result.length(), 0);
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::string err = "DB Error";
        send(clientSocket, err.c_str(), err.length(), 0);
    }
}

void handleRecentChats(const std::string& msg, SOCKET clientSocket) {
    std::string result = "";

    size_t pos1 = msg.find(":", 12); // RECENTCHATS:

    if (msg.length() <= 12) {
        std::string error = "Invalid Format";
        send(clientSocket, error.c_str(), error.length(), 0);
        closesocket(clientSocket);
        return;
    }

    std::string username = msg.substr(12);

    try {
        std::unique_ptr<sql::PreparedStatement> selectQuery( // �α��� ���� ���� Ȯ��
            conn->prepareStatement("SELECT user_id FROM users WHERE username = ?")
        );
        selectQuery->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(selectQuery->executeQuery());

        if (res->next()) {

            int user_id = res->getInt("user_id");

            std::unique_ptr<sql::PreparedStatement> psmt( // �α��� ��� �����
                conn->prepareStatement("SELECT * FROM message_log WHERE sender_id = ? ORDER BY message_id DESC LIMIT 10")
            );
            psmt->setInt(1, user_id);

            std::unique_ptr<sql::ResultSet> res(psmt->executeQuery());

            while (res->next()) { // ���� ������. ������ ���� ����
                result += "[" + std::to_string(res->getInt("message_id")) + "]" // result�� ���ڵ� �ϳ��� �߰�
                    + res->getString("content") + "/"
                    + res->getString("sent_at") + "/\n";
            }
        }
        else {
            result = "Can't find user";
        }
        send(clientSocket, result.c_str(), result.length(), 0);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::string err = "DB Error";
        send(clientSocket, err.c_str(), err.length(), 0);
    }
}

void handleActivity(const std::string& msg, SOCKET clientSocket) {
    std::string result = "";

    size_t pos1 = msg.find(":", 9); // ACTIVITY:

    if (msg.length() <= 9) {
        std::string error = "Invalid Format";
        send(clientSocket, error.c_str(), error.length(), 0);
        closesocket(clientSocket);
        return;
    }

    std::string username = msg.substr(9);

    try {
        std::unique_ptr<sql::PreparedStatement> selectQuery( // �α��� ���� ���� Ȯ��
            conn->prepareStatement("SELECT user_id FROM users WHERE username = ?")
        );
        selectQuery->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(selectQuery->executeQuery());

        if (res->next()) {

            int user_id = res->getInt("user_id");

            std::unique_ptr<sql::PreparedStatement> selectQuery2( // �޼��� ���� ����
                conn->prepareStatement("SELECT COUNT(content) AS message_count FROM message_log WHERE sender_id = ?")
            );
            selectQuery2->setInt(1, user_id);

            std::unique_ptr<sql::ResultSet> res2(selectQuery2->executeQuery());

            while (res2->next()) { // ���� ������. ������ ���� ����
                result += "user " + username + "posted "
                    +"[" + std::to_string(res2->getInt("message_count")) + "]" + "chats, "; // result�� �޼��� ���� �Է�
            }

            std::unique_ptr<sql::PreparedStatement> selectQuery3( // �α��� ��� ã��
                conn->prepareStatement("SELECT login_time FROM user_sessions WHERE user_id = ? ORDER BY session_id DESC LIMIT 1")
            );
            selectQuery3->setInt(1, user_id);

            std::unique_ptr<sql::ResultSet> res3(selectQuery3->executeQuery());

            while (res3->next()) { // ���� ������. ������ ���� ����
                result += "and the most recent login access was [" 
                    + res3->getString("login_time") + "]"; // result�� �α��� �ð� �Է�
            }
        }
        else {
            result = "Can't find user";
        }
        send(clientSocket, result.c_str(), result.length(), 0);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::string err = "DB Error";
        send(clientSocket, err.c_str(), err.length(), 0);
    }
}

void handleClient(SOCKET clientSocket) { // ������ Ŭ���̾�忡 ���� ���� �ɼǵ� ó��
    std::vector<char> buffer(1024);
    while (true) {
        int recvLen = recv(clientSocket, buffer.data(), buffer.size(), 0);

        if (recvLen <= 0) { // Ŭ���̾�Ʈ ���� ����� ���� Ż��
            std::cout << "Client Disconnect" << std::endl;
            break;
        }

        std::string msg(buffer.data(), recvLen);
        std::cout << "[RECV] " << msg << std::endl;

        if (msg.rfind("REGISTER:", 0) == 0) {
            handleRegister(msg, clientSocket); // ȸ������ ����
        }
        else if (msg.rfind("LOGIN:", 0) == 0) {
            handleLogin(msg, clientSocket); // �α��� ����
        }
        else if (msg.rfind("LOGOUT", 0) == 0) {
            handleLogout(msg, clientSocket); // �α׾ƿ� ����
        }
        else if (msg.rfind("CHAT:", 0) == 0) {
            handleChat(msg, clientSocket); // �α׾ƿ� ����
        }
        else if (msg.rfind("RECENTCHATS:", 0) == 0) {
            handleRecentChats(msg, clientSocket); // ������� �ֱ� ��ȭ 10�� �����ֱ�
        }
        else if (msg.rfind("ACTIVITY:", 0) == 0) {
            handleActivity(msg, clientSocket); // ������� �ֱ� ��ȭ 10�� �����ֱ�
        }
        else {
            std::string error = "Unknown Command";
            send(clientSocket, error.c_str(), error.length(), 0);
        }
    }
    closesocket(clientSocket); // ���� ������ ���� ����
}

int main() {

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket failed" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));


    listen(serverSocket, SOMAXCONN);
    std::cout << "[Server start] Listen in 9000..." << std::endl;

    if (!serverConnect()) { // MySQL�� ��й�ȣ�� DB�� �Է¹޾� ���� ����
        return 1;
    }

    SOCKET clientSocket;
    while (true) { // ���� ��� ����
        sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);

        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
        if (clientSocket != INVALID_SOCKET) {
            std::cout << "Client Connect" << std::endl;
            handleClient(clientSocket);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}