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

struct DataPacket {
    char name[32];
    int age;
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

    char buffer[256];

    while (true) {

        int len = recv(client_sock, buffer, 256, 0);
        if (len <= 0) break;
        buffer[len] = '\0';

        printf("Received message from client: %s\n", buffer);

        if (strcmp(buffer, "GetInfo") == 0) {
            DataPacket message = {"Jack", 22};
            int ret = send(client_sock, (const void *)&message, sizeof(DataPacket), 0);
            if (ret == -1) perror("send()");
        } else {
            char message[] = "unidentified cmd!";
            int ret = send(client_sock, message, strlen(message) + 1, 0);
            if (ret == -1) perror("send()"); 
        }
    }

    close(client_sock);
    close(sock);
 
    return 0;
    
}