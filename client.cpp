#include "EasyTcpClient.hpp"
#include "MessageHeader.hpp"

using namespace std;

const int N = 1000;      // number of clients
const int tCount = 2;   // number of threads

bool flag = true;

EasyTcpClient *client[N];

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

void SendThread(int id) {

    cout << "Thread " << id << " starts!" << endl;

    int segment = N / tCount;
    int begin = (id - 1) * segment;
    int end = begin + segment;

    for (int i = begin; i < end; i++)
        client[i] = new EasyTcpClient();
    
    for (int i = begin; i < end; i++) 
        client[i]->Connect("127.0.0.1", 4567);
    
    printf("thread<%d> Connect <begin = %d, end = %d>\n", id, begin, end);

    Login login;
    strcpy(login.Name, "hzh");
    strcpy(login.Password, "123456");
    
    while (flag) 
        for (int i = begin; i < end; i++)
            client[i]->SendData(&login);
    

    for (int i = begin; i < end; i++)
        client[i]->Close();

}

int main() {

    thread tid(CommandLine);
    tid.detach();

    for (int i = 1; i <= tCount; i++) {
        thread tid(SendThread, i);
        tid.detach();
    }

    while (flag)
        sleep(10);

    return 0;
}