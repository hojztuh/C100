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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        perror("connect()");
    
    char buffer[256];

    while (true) {
        printf("Input cmd!(Login, Logout, Exit)\n");
        // cmd: Login, Logout, Exit
        scanf("%s", buffer);

        if (strcmp(buffer, "Exit") == 0) break;
        else if (strcmp(buffer, "Login") == 0) {
            Header header = {sizeof(Login), CMD_LOGIN};
            Login login = { "hzh", "123456" };

            // send message to server
            send(sock, &header, sizeof(Header), 0);
            send(sock, &login, sizeof(Login), 0);

            // receive message from server
            Header hd;
            LoginResult result;
            recv(sock, &hd, sizeof(Header), 0);
            recv(sock, &result, sizeof(LoginResult), 0);

            // output result
            printf("Result is %d\n", result.result);
        } else if (strcmp(buffer, "Logout") == 0) {
            Header header = {sizeof(Logout), CMD_LOGOUT};
            Logout logout = { "hzh" };

            // send message to server
            send(sock, &header, sizeof(Header), 0);
            send(sock, &logout, sizeof(Logout), 0);

            // receive message from server
            Header hd;
            LogoutResult result;
            recv(sock, &hd, sizeof(Header), 0);
            recv(sock, &result, sizeof(LogoutResult), 0);

            // output result
            printf("Result is %d\n", result.result);
        } else printf("Please input the correct cammand!\n");

    }

    close(sock);

    return 0;
}