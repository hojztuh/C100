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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

struct Header {
    int Length;
    int cmd;
};

struct Login {
    char Name[32];
    char Password[32];
};

struct LoginResult {
    int result;
};

struct Logout {
    char Name[32];
};

struct LogoutResult {
    int result;
};



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

    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_sock = accept(sock, (sockaddr*)&client_addr, &client_len);
    if (client_sock == -1) perror("accept()");

    printf("Accept client! Client IP: %s\n", inet_ntoa(client_addr.sin_addr));

    while (true) {

        Header header;

        int len = recv(client_sock, &header, sizeof(Header), 0);    // receive header
        if (len <= 0) break;

        printf("Received header from client: length = %d, cmd = %d\n", header.Length, header.cmd);

        switch (header.cmd) {
            case CMD_LOGIN: {
                Login login;
                recv(client_sock, &login, sizeof(Login), 0);      // recv login info
                // omit authentication of username and password 
                Header hd = {sizeof(LoginResult), CMD_LOGIN_RESULT};
                send(client_sock, &hd, sizeof(Header), 0);  // send header
                LoginResult loginresult = { 0 };
                send(client_sock, &loginresult, sizeof(LoginResult), 0);    // send login result
                break;
            }
            case CMD_LOGOUT: {
                Logout logout;
                recv(client_sock, &logout, sizeof(Logout), 0);      // recv logout info
                // omit authentication of username
                Header hd = {sizeof(LogoutResult), CMD_LOGOUT_RESULT};
                send(client_sock, &hd, sizeof(Header), 0);  // send header
                LogoutResult logoutresult = { 1 };
                send(client_sock, &logoutresult, sizeof(LogoutResult), 0);    // send login result
                break;
            }
            default: {
                Header hd = { 0, CMD_ERROR};
                send(client_sock, &hd, sizeof(Header), 0);
                break;
            }
        }
    }

    close(client_sock);
    close(sock);
 
    return 0;
    
}