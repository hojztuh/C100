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
#include <thread>
#include "MessageHeader.hpp"

#define INVALID_SOCKET (-1)

class EasyTcpServer {

private:
    int sock;
    std::vector<int> g_clients;

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
            NewUserJoin message;
            message.sock = client_sock;
            SendDataToAll(&message); 

            g_clients.push_back(client_sock);
            
            printf("Accept client! Client IP: %s\n", inet_ntoa(client_addr.sin_addr));

        }

        return client_sock;
    }

    // Process Network Message

    bool OnRun() {

        if (!isRun()) return false;

        fd_set fdRead, fdWrite, fdExcept;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExcept);

        FD_SET(sock, &fdRead);  // insert sock to monitor

        for (int i = 0; i < g_clients.size(); i++) FD_SET(g_clients[i], &fdRead);   // monitor the clients

        struct timeval t = {1, 0};
        
        int ret = select(FD_SETSIZE, &fdRead, &fdWrite, &fdExcept, &t);
        if (ret < 0) {
            perror("select()");
            Close();
            return false;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, &fdRead)) {

                if (i == sock) {    // accept
                    FD_CLR(sock, &fdRead);  // delete sock
                    Accept();

                } else {
                    int ret = RecvData(i);
                    if (ret < 0) {  // client Exit
                        close(i);
                        auto iter = find(g_clients.begin(), g_clients.end(), i);
                        if (iter != g_clients.end()) g_clients.erase(iter);
                    }
                    FD_CLR(i, &fdRead);
                }
            }
        
        printf("Process other business in the spare time\n");

        return true;
    }

    bool isRun() {
        return sock != INVALID_SOCKET;
    }

    int RecvData(int client_sock) {

        char buffer[1024];

        int len = recv(client_sock, buffer, sizeof(Header), 0);    // receive header
        if (len <= 0) {
            printf("Client<%d> Exit!\n", client_sock);
            return -1;
        }

        Header *header = (Header *)buffer;

        recv(client_sock, buffer + sizeof(Header), header->Length - sizeof(Header), 0);

        OnNetMsg(client_sock, header);

        return 0; 
    }

    // respond to network message
    void OnNetMsg(int client_sock, Header *header) {

        printf("Received header from client<%d>: length = %d, cmd = %d\n", client_sock, header->Length, header->cmd);

        switch (header->cmd) {
            case CMD_LOGIN: {
                Login *login = (Login *)header;
                printf("Name:%s\nPassword: %s\n", login->Name, login->Password);
                // omit authentication of username and password 
                LoginResult result;
                result.result = 0;
                send(client_sock, &result, sizeof(LoginResult), 0);  // send result
                break;
            }
            case CMD_LOGOUT: {
                Logout *logout = (Logout *)header;
                printf("Name:%s\n", logout->Name);
                // omit authentication of username
                LogoutResult result;
                result.result = 1;
                send(client_sock, &result, sizeof(LogoutResult), 0);  // send result
                break;
            }
            default: {
                Header hd = { 0, CMD_ERROR};
                send(client_sock, &hd, sizeof(Header), 0);
                break;
            }
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
        for (int i = 0; i < g_clients.size(); i++) 
            SendData(g_clients[i], header);
    }

    // close socket
    void Close() {
        if (sock != INVALID_SOCKET) {
            for (int i = 0; i < g_clients.size(); i++) close(g_clients[i]);
            close(sock);
            sock = INVALID_SOCKET;
        }
    }


};

#endif