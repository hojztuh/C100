#include "EasyTcpClient.hpp"
#include "MessageHeader.hpp"

using namespace std;

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
    client.Connect("127.0.0.1", 4567);

    thread tid(CommandLine, &client);
    tid.detach();
    
    while (client.isRun()) {

        client.OnRun();
    }

    client.Close();


    return 0;
}