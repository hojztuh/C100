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

int Process(int _sock) {

    char buffer[1024];

    int len = recv(_sock, buffer, sizeof(Header), 0);    // receive header
    if (len <= 0) {
        printf("Disconnected from server!\n");
        return -1;
    }

    Header *header = (Header *)buffer;

    printf("Received header from server: length = %d, cmd = %d\n", header->Length, header->cmd);

    switch (header->cmd) {
        case CMD_LOGIN_RESULT: {
            recv(_sock, buffer + sizeof(Header), header->Length - sizeof(Header), 0);
            LoginResult *result = (LoginResult *)buffer;
            printf("LoginResult: %d\n", result->result);
            break;
        }
        case CMD_LOGOUT_RESULT: {
            recv(_sock, buffer + sizeof(Header), header->Length - sizeof(Header), 0);
            LogoutResult *result = (LogoutResult *)buffer;
            printf("LogoutResult: %d\n", result->result);
            break;
        }
        case CMD_NEW_USER_JOIN: {
            recv(_sock, buffer + sizeof(Header), header->Length - sizeof(Header), 0);
            NewUserJoin *message = (NewUserJoin *)buffer;
            printf("The new joined client's sock is %d\n", message->sock);
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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        perror("connect()");
    
    while (true) {

        fd_set fd_Read;
        FD_ZERO(&fd_Read);
        FD_SET(sock, &fd_Read);

        struct timeval t = {1, 0};

        int ret = select(FD_SETSIZE, &fd_Read, NULL, NULL, &t);

        if (ret < 0) {
            perror("select()");
            break;
        }

        if (FD_ISSET(sock, &fd_Read)) {
            FD_CLR(sock, &fd_Read);
            if (Process(sock) < 0) 
                break;
        } 

        printf("Process other business in the spare time\n");

        // send message to server
        Login login;
        strcpy(login.Name, "hzh");
        strcpy(login.Password, "123456");
        send(sock, &login, sizeof(Login), 0);
        sleep(1);

    }

    close(sock);

    return 0;
}