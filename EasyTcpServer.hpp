#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <functional>
#include <atomic>
#include <thread>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef RECV_BUF_SIZE
#define RECV_BUF_SIZE 4096
#endif

#ifndef CELL_THREAD_COUNT
#define CELL_THREAD_COUNT 2
#endif

int cnt;

class ClientSocket {
private:
    int sockfd; 
    char msgbuffer[RECV_BUF_SIZE * 10]; // message buffer
    int LastPos;                        // the tail of the message buffer

public:
    ClientSocket(int sock = INVALID_SOCKET) {
        sockfd = sock;
        memset(msgbuffer, 0, sizeof(msgbuffer));
        LastPos = 0;
    }

    virtual ~ClientSocket() {

    }

    int SockFd() {
        return sockfd;
    }

    char *MsgBuffer() {
        return msgbuffer;
    }

    int GetLastPos() {
        return LastPos;
    }

    void SetLastPos(int pos) {
        LastPos = pos;
    }

};

class INetEvent {
public:
    virtual void OnLeave(ClientSocket *pClient) = 0;
private:

};

class CellServer {

private:
    int sock;
    std::vector<ClientSocket*> clients;
    std::vector<ClientSocket*> clientsBuff;
    std::thread tid;
    char RecvBuffer[RECV_BUF_SIZE];     // receive buffer
    std::mutex mtx;
    INetEvent *pNetEvent;
public:
    std::atomic_int RecvCount;

public:
    CellServer(int _sock = INVALID_SOCKET) {
        sock = _sock;
        RecvCount = 0;
        pNetEvent = nullptr;
    }

    ~CellServer() {
        Close();
    }

    void setEventObj(INetEvent *event) {
        pNetEvent = event;
    }

    bool OnRun() {

        while (isRun()) {

            if (clientsBuff.size()) {

                std::lock_guard<std::mutex> lock(mtx);

                for (auto item : clientsBuff)
                    clients.push_back(item);
                clientsBuff.clear();

            }

            if (clients.empty()) {
                std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
                continue;
            }

            fd_set fdRead, fdWrite, fdExcept;

            FD_ZERO(&fdRead);
            FD_ZERO(&fdWrite);
            FD_ZERO(&fdExcept);

            for (int i = 0; i < clients.size(); i++) FD_SET(clients[i]->SockFd(), &fdRead);   // monitor the clients

            int ret = select(FD_SETSIZE, &fdRead, &fdWrite, &fdExcept, nullptr);
            if (ret < 0) {
                perror("select()");
                Close();
                return false;
            }

            for (int i = 0; i < FD_SETSIZE; i++)
                if (FD_ISSET(i, &fdRead)) {
    
                    ClientSocket *pClient;
                    for (int j = 0; j < clients.size(); j++) 
                        if (clients[j]->SockFd() == i)
                            pClient = clients[j];
                    int ret = RecvData(pClient);
                    if (ret < 0) {  // client Exit
                        close(i);
                        if (pNetEvent)
                            pNetEvent->OnLeave(pClient);
                        delete pClient;
                        clients.erase(find(clients.begin(), clients.end(), pClient));
                    }
                    FD_CLR(i, &fdRead); 
                }  
        }

        return false;
    }

    bool isRun() {
        return sock != INVALID_SOCKET;
    }

    // close socket
    void Close() {
        if (sock != INVALID_SOCKET) {
            for (int i = 0; i < clients.size(); i++) {
                close(clients[i]->SockFd());
                delete clients[i];
            }
            close(sock);
            sock = INVALID_SOCKET;
            clients.clear();
        }
    }

    int RecvData(ClientSocket *pClient) {

        int len = recv(pClient->SockFd(), RecvBuffer, RECV_BUF_SIZE, 0);
        if (len <= 0) {
            printf("Disconnected from server!\n");
            return -1;
        }

        // copy data from RecvBuffer to MsgBuffer
        memcpy(pClient->MsgBuffer() + pClient->GetLastPos(), RecvBuffer, len);
        // move poiter
        pClient->SetLastPos(pClient->GetLastPos() + len);

        while (pClient->GetLastPos() >= sizeof(Header)) {
            Header *header = (Header *)pClient->MsgBuffer();
            if (pClient->GetLastPos() >= header->Length) {    // can process one datagram
                int remain = pClient->GetLastPos() - header->Length;
                OnNetMsg(pClient->SockFd(),header);   // process the current datagram
                memcpy(pClient->MsgBuffer(), pClient->MsgBuffer() + header->Length, remain);
                //LastPos = remain;
                pClient->SetLastPos(remain);
            } else break;   // less than one datagram
        }

        return 0; 
    }

        // respond to network message
    void OnNetMsg(int client_sock, Header *header) {
        RecvCount++;
        // auto t1 = tTime.GetElapsedSecond();
        // if (t1 >= 1.0) {
        //     printf("time<%f>, clients<%ld>, RecvCount<%d>\n", t1, clients.size(), RecvCount);
        //     RecvCount = 0;
        //     tTime.update();
        // }

        // printf("Received header from client<%d>: length = %d, cmd = %d\n", client_sock, header->Length, header->cmd);

        switch (header->cmd) {
            case CMD_LOGIN: {
                // Login *login = (Login *)header;
                // printf("Name:%s\nPassword: %s\n", login->Name, login->Password);
                // omit authentication of username and password 
                // LoginResult result;
                // result.result = 0;
                // send(client_sock, &result, sizeof(LoginResult), 0);  // send result
                break;
            }
            case CMD_LOGOUT: {
                // Logout *logout = (Logout *)header;
                // printf("Name:%s\n", logout->Name);
                // omit authentication of username
                // LogoutResult result;
                // result.result = 1;
                // send(client_sock, &result, sizeof(LogoutResult), 0);  // send result
                break;
            }
            default: {
                Header hd = { 0, CMD_ERROR};
                send(client_sock, &hd, sizeof(Header), 0);
                break;
            }
        }
    }

    void addClient(ClientSocket* pClient) {
        std::lock_guard<std::mutex> lock(mtx);
        clientsBuff.push_back(pClient);
    }

    int GetClientCount() {
        return clients.size() + clientsBuff.size();
    }

    void Start() {        
        tid = std::thread(std::mem_fn(&CellServer::OnRun), this);
    }

};

class EasyTcpServer : public INetEvent {

private:
    int sock;
    std::vector<ClientSocket*> clients;
    std::vector<CellServer*> cellServers;
    CELLTimestamp tTime;
    int RecvCount;      // the number of the received datagrams

public:

    EasyTcpServer() {
        sock = INVALID_SOCKET;
    }

    virtual ~EasyTcpServer() {
        Close();
    }

    // initialize socket
    void InitSocket() {
        if (sock != INVALID_SOCKET)
            Close();
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) 
            perror("socket()");
    }

    // bind 
    int Bind(const char *ip, unsigned short port) {

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = ip ? inet_addr(ip) : INADDR_ANY;

        int ret = bind(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret == -1)
            perror("bind()");
        
        return ret;
    }

    // listen
    int Listen(int n) {
        int ret = listen(sock, n);
        if (ret < 0) perror("listen()");

        return ret;
    }

    // accept connection

    int Accept() {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_sock = accept(sock, (sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) perror("accept()");
        else {
            // NewUserJoin message;
            // message.sock = client_sock;
            // SendDataToAll(&message); 

            AddClientToCellServer(new ClientSocket(client_sock));
            
            // cnt++;
            // printf("Accept client<%d>! Client IP: %s\n", cnt, inet_ntoa(client_addr.sin_addr));

        }

        return client_sock;
    }

    void AddClientToCellServer(ClientSocket *pClient) {
        clients.push_back(pClient);

        auto pMinServer = cellServers[0];

        for (auto pCellServer : cellServers) 
            if (pCellServer->GetClientCount() < pMinServer->GetClientCount()) 
                pMinServer = pCellServer;
        pMinServer->addClient(pClient);
    }


    void Start() {

        for (int i = 0; i < CELL_THREAD_COUNT; i++) {
            auto ser = new CellServer(sock);
            cellServers.push_back(ser);
            ser->setEventObj(this);
            ser->Start();
        }
    }

    // Process Network Message

    bool OnRun() {

        if (!isRun()) return false;

        time4msg();

        fd_set fdRead, fdWrite, fdExcept;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExcept);

        FD_SET(sock, &fdRead);  // insert sock to monitor
        
        struct timeval t = {0, 10};
        
        int ret = select(FD_SETSIZE, &fdRead, &fdWrite, &fdExcept, &t);

        if (ret < 0) {
            perror("select()");
            Close();
            return false;
        }

        if (FD_ISSET(sock, &fdRead)) {

            FD_CLR(sock, &fdRead);  // delete sock
            Accept();
        }
                 
        return true;
    }

    bool isRun() {
        return sock != INVALID_SOCKET;
    }

    void time4msg() {

        RecvCount++;
        auto t1 = tTime.GetElapsedSecond();
        if (t1 >= 1.0) {
            RecvCount = 0;
            for (auto ser : cellServers) {
                RecvCount += ser->RecvCount;
                ser->RecvCount = 0;
            }
            printf("thread<%ld>, time<%f>, clients<%ld>, RecvCount<%d>\n", cellServers.size(), t1, clients.size(), (int)(RecvCount/t1));
            RecvCount = 0;
            tTime.update();
        }
    }

    // Send data to specific client
    int SendData(int client_sock, Header *header) {
        if (isRun() && header)
            return send(client_sock, header, header->Length, 0);
        return -1;
    }

    // Send data to all the connected client
    void SendDataToAll(Header *header) {
        // send the new client join message to all the connected clients
        for (int i = 0; i < clients.size(); i++) 
            SendData(clients[i]->SockFd(), header);
    }

    // close socket
    void Close() {
        if (sock != INVALID_SOCKET) {
            for (int i = 0; i < clients.size(); i++) {
                close(clients[i]->SockFd());
                delete clients[i];
            }
            close(sock);
            sock = INVALID_SOCKET;
            clients.clear();
        }
    }

    virtual void OnLeave(ClientSocket *pClient) {
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i] == pClient) {
                auto iter = clients.begin() + i;
                if (iter != clients.end())
                    clients.erase(iter);
            }
        }
    }


};

#endif