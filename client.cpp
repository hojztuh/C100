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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        perror("connect()");
    
    char buffer[256];

    while (true) {
        printf("Input cmd!\n");
        // cmd: GetInfo, Exit
        scanf("%s", buffer);

        if (strcmp(buffer, "Exit") == 0) break;

        int ret = send(sock, buffer, strlen(buffer) + 1, 0);
        if (ret == -1) perror("send()");

        int len = recv(sock, buffer, 256, 0);

        // buffer can be a struct or "unidentified cmd!"(string)

        DataPacket *ptr = (DataPacket *)buffer;

        printf("Received:\nName: %s\nAge: %d\n", ptr->name, ptr->age);

    }

    close(sock);

    return 0;
}