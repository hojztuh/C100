#include "EasyTcpServer.hpp"
#include "MessageHeader.hpp"

using namespace std;

bool flag = true;

void CommandLine() {

    while (true) {
        char buffer[256];

        scanf("%s", buffer);

        if (strcmp(buffer, "Exit") == 0) {
            flag = false;
            printf("Exit!\n");
            break;
        } 
    }
}

int main() {
 
    EasyTcpServer server;

    server.InitSocket();
    server.Bind(NULL, 4567);
    server.Listen(5);
    server.Start();

    thread tid(CommandLine);
    tid.detach();

    while (server.isRun() && flag) 
        server.OnRun();
    server.Close();

    return 0;
    
}