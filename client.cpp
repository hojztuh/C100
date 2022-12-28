#include "EasyTcpClient.hpp"
#include "MessageHeader.hpp"

using namespace std;

bool flag = true;

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

void CommandLine(EasyTcpClient *client) {

    while (true) {
        char buffer[256];

        scanf("%s", buffer);

        if (strcmp(buffer, "Login") == 0) {
            Login login;
            strcpy(login.Name, "hzh");
            strcpy(login.Password, "123456");
            client->SendData(&login);
        } else if (strcmp(buffer, "Logout") == 0) {
            Logout logout;
            strcpy(logout.Name, "hzh");
            client->SendData(&logout);
        } else if (strcmp(buffer, "Exit") == 0) {
            client->Close();
            printf("Exit!\n");
            break;
        } else printf("Please input correct cmd!(Login, Logout, Exit)\n");

    }
}

int main() {
    
    EasyTcpClient client;

    client.InitSocket();
    client.Connect("127.0.0.1", 4444);


    thread tid(CommandLine, &client);
    tid.detach();
    
    while (client.isRun()) {

        client.OnRun();
    }

    client.Close();


    return 0;
}