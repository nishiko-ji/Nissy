#include "game.h"
#include "Naotti/AI.h"
#include "Naotti/BitBoard.h"

namespace game
{
    // sの先頭がt ⇔ true
    bool startWith(string &s, string t)
    {
        for (int i = 0; i < t.length(); i++)
        {
            if (i >= s.length() || s[i] != t[i])
                return false;
        }
        return true;
    }

    //ゲームの終了判定
    int isEnd(string s)
    {
        if (startWith(s, "WON"))
            return WON;
        if (startWith(s, "LST"))
            return LST;
        if (startWith(s, "DRW"))
            return DRW;
        return 0;
    }

    // 赤駒の配置(ランダム)
    string setPosition()
    {
        string redName = "ABCDEFGH";
        random_device seed_gen;
        mt19937 engine(seed_gen());
        shuffle(redName.begin(), redName.end(), engine);
        redName = redName.substr(4);
        sort(redName.begin(), redName.end());
        return "SET:" + redName;
    }

    void uct(UCT &Tree)
    {
        Tree.Search(); //探索
        std::cout << "-----Finish UCT Search-----" << std::endl;
        Tree.PrintStatus();
        std::cout << "---------------------------" << std::endl;
    }
    void minmax(pair<string, int> &mv, string recieve, int turn, int depth)
    {
        mv = tsumi(recieve, turn, depth);
        std::cout << "Finish 紫駒 depth:" << depth << " result " << mv.first << ", " << mv.second << std::endl;
    }
    // ゲームを行う
    int playgame(int port, string destination)
    {
        string position;
        int result;
        int turn = 0;
        Client client(port, destination);

        if (!client.openPort())
            return 0;
        Log log{};
        if (!log.openLog())
            return 0;

        position = setPosition();
        client.Recv();
        client.Send(position);
        client.Recv();

        const Recieve StartStr = "MOV?14R24R34R44R15B25B35B45B41u31u21u11u40u30u20u10u"; //初期盤面
        UCT Tree(StartStr);
        Recieve recieve;
        Send send;

        // minmax
        bb::prepare();

        pair<string, int> mv5;
        pair<string, int> mv7;
        pair<string, int> mv9;
        pair<string, int> mv11;
        pair<string, int> mv13;

        // 対戦
        while (true)
        {
            recieve = client.Recv();       // サーバーからの文字列を受け取る
            result = game::isEnd(recieve); // ゲームの終了判定
            if (result)
                break;             // 終了してたら繰り返し抜ける
            Tree.SetNode(recieve); //受け取った文字列通りにセット
            std::thread t1(uct, std::ref(Tree));
            std::thread t2(minmax, std::ref(mv5), recieve, turn, 5);
            std::thread t3(minmax, std::ref(mv7), recieve, turn, 7);
            std::thread t4(minmax, std::ref(mv9), recieve, turn, 9);
            std::thread t5(minmax, std::ref(mv11), recieve, turn, 11);
            std::thread t6(minmax, std::ref(mv13), recieve, turn, 13);
            t1.join();
            t2.join();
            t3.join();
            t4.join();
            t5.join();
            t6.join();
            if (mv13.first.find("NAN") == string::npos && mv13.second > 10000000)
            {
                send = mv13.first;
                cout << "SELECT : 紫駒13" << endl;
                cout << send << endl;
            }
            else if (mv11.first.find("NAN") == string::npos && mv11.second > 10000000)
            {
                send = mv11.first;
                cout << "SELECT : 紫駒11" << endl;
                cout << send << endl;
            }
            else if (mv9.first.find("NAN") == string::npos && mv9.second > 10000000)
            {
                send = mv9.first;
                cout << "SELECT : 紫駒9" << endl;
                cout << send << endl;
            }
            else if (mv7.first.find("NAN") == string::npos && mv7.second > 10000000)
            {
                send = mv7.first;
                cout << "SELECT : 紫駒7" << endl;
                cout << send << endl;
            }
            else if (mv5.first.find("NAN") == string::npos && mv5.second > 10000000)
            {
                send = mv5.first;
                cout << "SELECT : 紫駒5" << endl;
                cout << send << endl;
            }
            else
            {
                NodeNum move = Tree.Choice(); //探索結果に合わせてrootからノードを選択
                log.writeLog(Tree.getStatus(move));
                send = Tree.MoveNode(move); //選択したノードに遷移し、サーバーに送る文字列を受け取る
                cout << "SELECT : UCT" << endl;
                cout << send << endl;
            }
            client.Send(send); //サーバーに文字列を送る
            client.Recv();     // ACKの受信
            turn += 2;
        }

        if (result == WON)
            log.writeLog("WIN");
        if (result == DRW)
            log.writeLog("DRAW");
        if (result == LST)
            log.writeLog("LOSE");
        client.closePort();
        return result;
    }
}
