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
    char buf[] = "Hello, I am server!";

    while (true) {

        int client_sock = accept(sock, (sockaddr*)&client_addr, &client_len);
        if (client_sock == -1) perror("accept()");

    
        // write(client_sock, buf, strlen(buf) + 1);

        send(client_sock, buf, strlen(buf) + 1, 0);

        printf("send message to client!\n");

        close(client_sock);
    }


    // close(client_sock);
    close(sock);
 
    return 0;
    
}