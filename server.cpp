#include "EasyTcpServer.hpp"
#include "MessageHeader.hpp"

using namespace std;

int main() {
 
    EasyTcpServer server;

    server.InitSocket();
    server.Bind(NULL, 4444);
    server.Listen(5);

    while (server.isRun()) 
        server.OnRun();
    server.Close();

    return 0;
    
}