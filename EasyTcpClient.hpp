#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

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

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef RECV_BUF_SIZE
#define RECV_BUF_SIZE 4096
#endif

class EasyTcpClient {

private:
    int sock;
    char RecvBuffer[RECV_BUF_SIZE];     // receive buffer
    char MsgBuffer[RECV_BUF_SIZE * 10]; // message buffer
    int LastPos;                        // the tail of the message buffer

public:
    EasyTcpClient() {
        sock = INVALID_SOCKET;
        LastPos = 0;
    }

    virtual ~EasyTcpClient() {
        Close();
    }

    // initialize socket
    void InitSocket() {
        if (sock != INVALID_SOCKET) 
            Close();
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) 
            perror("socket()");
    }

    // connect to server
    void Connect(const char *ip, unsigned short port) {

        if (sock == INVALID_SOCKET)
            InitSocket();

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = inet_addr(ip);

        if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
            perror("connect()");
    }

    // close 
    void Close() {
        if (sock != INVALID_SOCKET) {
            close(sock);
            sock = INVALID_SOCKET;
        }
    }

    // process network message

    bool OnRun() {

        if (!isRun()) return false;

        fd_set fd_Read;
        FD_ZERO(&fd_Read);
        FD_SET(sock, &fd_Read);

        struct timeval t = {1, 0};

        int ret = select(FD_SETSIZE, &fd_Read, NULL, NULL, &t);

        if (ret < 0) {
            perror("select()");
            Close();
            return false;
        }

        if (FD_ISSET(sock, &fd_Read)) {
            FD_CLR(sock, &fd_Read);
            if (RecvData() < 0) {
                Close();
                return false;
            }
        } 

        return true;
    }

    // ???????????? ?????????

    int RecvData() {

        // int len = recv(sock, buffer, sizeof(Header), 0);    // receive header
        int len = recv(sock, RecvBuffer, RECV_BUF_SIZE, 0);
        if (len <= 0) {
            printf("Disconnected from server!\n");
            return -1;
        }

        // copy data from RecvBuffer to MsgBuffer
        memcpy(MsgBuffer + LastPos, RecvBuffer, len);
        // move poiter
        LastPos += len;

        while (LastPos >= sizeof(Header)) {
            Header *header = (Header *)MsgBuffer;
            if (LastPos >= header->Length) {    // can process one datagram
                int remain = LastPos - header->Length;
                OnNetMsg(header);   // process the current datagram
                memcpy(MsgBuffer, MsgBuffer + header->Length, remain);
                LastPos = remain;
            } else break;   // less than one datagram
        }

        return 0; 
    }

    // ??????????????????
    void OnNetMsg(Header *header) {

        printf("Received header from server: length = %d, cmd = %d\n", header->Length, header->cmd);

        switch (header->cmd) {
            case CMD_LOGIN_RESULT: {
                
                LoginResult *result = (LoginResult *)header;
                printf("LoginResult: %d\n", result->result);
                break;
            }
            case CMD_LOGOUT_RESULT: {
                LogoutResult *result = (LogoutResult *)header;
                printf("LogoutResult: %d\n", result->result);
                break;
            }
            case CMD_NEW_USER_JOIN: {
                NewUserJoin *message = (NewUserJoin *)header;
                printf("The new joined client's sock is %d\n", message->sock);
            }
        }
    }

    int SendData(Header *header) {
        if (isRun() && header)
            return send(sock, header, header->Length, 0);
        return -1;
    }


    bool isRun() {
        return sock != INVALID_SOCKET;
    }
};

#endif