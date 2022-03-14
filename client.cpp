#include "client.h"

Client::Client(int _port, std::string _ip)
{
    port = _port;
    ip = _ip;
}

bool Client::openPort()
{
    // IP アドレス，ポート番号，ソケット，sockaddr_in 構造体
    char destination[32];
    struct sockaddr_in dstAddr;
    int PORT;

    // Windows の場合
    // WSADATA data;
    // WSAStartup(MAKEWORD(2, 0), &data);

    // 相手先アドレスの入力と送る文字の入力
    if (port == -1)
    {
        std::cout << "ポート番号は？：";
        scanf("%d", &PORT);
    }
    else
        PORT = port;

    if (ip.length() == 0)
    {
        std::cout << "サーバーマシンのIPは？：";
        scanf("%s", destination);
    }
    else
    {
        for (int i = 0; i < ip.size(); i++)
            destination[i] = ip[i];
        destination[ip.size()] = '\0';
    }

    // sockaddr_in 構造体のセット
    memset(&dstAddr, 0, sizeof(dstAddr));
    dstAddr.sin_port = htons(PORT);
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_addr.s_addr = inet_addr(destination);

    dstSocket = socket(AF_INET, SOCK_STREAM, 0); // ソケットの生成

    //接続
    if (connect(dstSocket, (struct sockaddr *)&dstAddr, sizeof(dstAddr)))
    {
        std::cout << destination << " に接続できませんでした" << std::endl;
        return false;
    }
    std::cout << destination << " に接続しました" << std::endl;
    return true;
}

void Client::closePort()
{
    // Windows でのソケットの終了
    // closesocket(dstSocket);
    // WSACleanup();

    // linuxでのソケットの終了
    close(dstSocket);
}

void Client::Send(std::string str)
{
    if (str.length() == 0)
    { // null文字なら「入力」を受け付ける
        std::cin >> str;
    }
    if (str.length() < 2 || str[str.length() - 2] != '\r' || str[str.length() - 1] != '\n')
    { //\rが末尾になければ追加
        str += '\r';
        str += '\n';
    }
    int byte = send(dstSocket, str.c_str(), str.length(), 0); //文字列を送信
    if (byte <= 0)
    {
        std::cout << "送信エラー" << std::endl;
    }
}

std::string Client::Recv()
{
    char buffer[10];
    std::string msg;

    do
    {
        int byte = recv(dstSocket, buffer, 1, 0); //文字を受信
        if (byte == 0)
            break;
        if (byte < 0)
        {
            std::cout << "受信に失敗しました" << std::endl;
            return msg;
        }

        msg += buffer[0];
    } while (msg.length() < 2 || msg[msg.length() - 2] != '\r' || msg[msg.length() - 1] != '\n');
    std::cout << "受信 = " << msg << std::endl;
    return msg;
}

Client::~Client()
{
    closePort();
}