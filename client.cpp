#include "EasyTcpClient.hpp"
#include "MessageHeader.hpp"

using namespace std;

const int N = 2000;

bool flag = true;

void CommandLine() {

    while (true) {
        char buffer[256];
        scanf("%s", buffer);
        if (strcmp(buffer, "Exit") == 0) {
            flag = false;
            printf("Exit!\n");
            break;
        } else printf("Please input correct cmd!(Exit)\n");
    }
}

int main() {

    EasyTcpClient *client[N];

    for (int i = 0; i < N; i++)
        client[i] = new EasyTcpClient();
    
    thread tid(CommandLine);
    tid.detach();
    
    for (int i = 0; i < N; i++) {
        if (!flag) return 0;
        client[i]->Connect("127.0.0.1", 4567);
        printf("client<%d> connected!\n", i + 1);
    }

    Login login;
    strcpy(login.Name, "hzh");
    strcpy(login.Password, "123456");
    
    while (flag) {
        for (int i = 0; i < N; i++)
            client[i]->SendData(&login);
    }

    for (int i = 0; i < N; i++)
        client[i]->Close();

    return 0;
}