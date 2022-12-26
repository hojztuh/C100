/*
    How to solve the problem of "Bind(): Address already in use?"
    1. $netstat -tulpn | less
    2. find the pid of the process
    3. $kill -9 pid
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// #define FD_SETSIZE 1024

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

struct Header {
    int Length;
    int cmd;
};

struct Login: public Header {

    Login() {
        Length = sizeof(Login);
        cmd = CMD_LOGIN;
    }

    char Name[32];
    char Password[32];
};

struct LoginResult: public Header {
    LoginResult() {
        Length = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
    }

    int result;
};

struct Logout: public Header {

    Logout() {
        Length = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }

    char Name[32];
};

struct LogoutResult: public Header {
    
    LogoutResult() {
        Length = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
    }

    int result;
};

struct NewUserJoin:public Header {

    NewUserJoin() {
        Length = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }

    int sock;

};

std::vector<int> g_clients;

int Process(int client_sock) {

    char buffer[1024];

    int len = recv(client_sock, buffer, sizeof(Header), 0);    // receive header
    if (len <= 0) {
        printf("Client<%d> Exit!\n", client_sock);
        return -1;
    }

    Header *header = (Header *)buffer;

    printf("Received header from client<%d>: length = %d, cmd = %d\n", client_sock, header->Length, header->cmd);

    switch (header->cmd) {
        case CMD_LOGIN: {
            recv(client_sock, buffer + sizeof(Header), header->Length - sizeof(Header), 0);      // recv login info
            Login *login = (Login *)buffer;
            printf("Name:%s\nPassword: %s\n", login->Name, login->Password);
            // omit authentication of username and password 
            LoginResult result;
            result.result = 0;
            send(client_sock, &result, sizeof(LoginResult), 0);  // send result
            break;
        }
        case CMD_LOGOUT: {
            recv(client_sock, buffer + sizeof(Header), header->Length - sizeof(Header), 0);      // recv logout info
            Logout *logout = (Logout *)buffer;
            printf("Name:%s\n", logout->Name);
            // omit authentication of username
            LogoutResult result;
            result.result = 1;
            send(client_sock, &result, sizeof(LogoutResult), 0);  // send result
            break;
        }
        default: {
            Header hd = { 0, CMD_ERROR};
            send(client_sock, &hd, sizeof(Header), 0);
            break;
        }
    }

    return 0; 
}

int main() {

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) 
        perror("socket()");

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4567);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        perror("bind()");
    
    if (listen(sock, 5) == -1)
        perror("listen()");

    while (true) {

        fd_set fdRead, fdWrite, fdExcept;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExcept);

        FD_SET(sock, &fdRead);  // insert sock to monitor

        for (int i = 0; i < g_clients.size(); i++) FD_SET(g_clients[i], &fdRead);   // monitor the clients

        struct timeval t = {1, 0};
        
        int ret = select(FD_SETSIZE, &fdRead, &fdWrite, &fdExcept, &t);
        if (ret < 0) {
            perror("select()");
            break;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, &fdRead)) {

                if (i == sock) {    // accept
                    FD_CLR(sock, &fdRead);  // delete sock

                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
            
                    int client_sock = accept(sock, (sockaddr*)&client_addr, &client_len);
                    if (client_sock == -1) perror("accept()");
                    else {
                        // send the new client join message to all the connected clients

                        for (int i = 0; i < g_clients.size(); i++) {
                            NewUserJoin message;
                            message.sock = client_sock;
                            send(g_clients[i], &message, sizeof(NewUserJoin), 0);  // send result 
                        }

                        g_clients.push_back(client_sock);
                        
                        printf("Accept client! Client IP: %s\n", inet_ntoa(client_addr.sin_addr));

                    }

                    

                } else {
                    int ret = Process(i);
                    if (ret < 0) {  // client Exit
                        close(i);
                        auto iter = find(g_clients.begin(), g_clients.end(), i);
                        if (iter != g_clients.end()) g_clients.erase(iter);
                    }
                    FD_CLR(i, &fdRead);
                }
            }
        
        printf("Process other business in the spare time\n");

    }

    for (int i = 0; i < g_clients.size(); i++) close(g_clients[i]);
    // close(client_sock);
    close(sock);
 
    return 0;
    
}