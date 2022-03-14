#pragma once

// visualstudioでのtcp通信で必要
// #pragma comment(lib, "wsock32.lib")
// #include <winsock2.h>
// #include <ws2tcpip.h>

// linuxでのtcp通信で必要
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// OS共通
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

class Client
{
private:
    int dstSocket;  // socket
    int port;       // ポート番号
    std::string ip; // IPアドレス

public:
    Client(int _port = -1, std::string _ip = ""); // コンストラクタ
    bool openPort();                              // ポートを開く
    void closePort();                             // ポートを閉じる
    void Send(std::string str = "");              // サーバに文字列を送る
    std::string Recv();                           // サーバから文字列を受け取る
    ~Client();                                    // デストラクタ
};
