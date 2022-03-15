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

    void uct(UCT& Tree){
        std::cout << "-----Start UCT Search-----" << std::endl;
        Tree.Search();         //探索
        std::cout << "-----Finish UCT Search-----" << std::endl;
        Tree.PrintStatus();
        std::cout << "---------------------------" << std::endl;
    }
    void minmax(pair<string, int>& mv, string recieve, int turn) {
        std::cout << "-----Start 紫駒 Search-----" << std::endl;
        mv = tsumi(recieve, turn);
        std::cout << "-----Finish 紫駒 Search-----" << std::endl;
        std::cout << mv.first << ", " << mv.second << std::endl; 
        std::cout << "----------------------------" << std::endl;
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

        //minmax
        bb::prepare();
        bb::weight1 = 1000;
        bb::weight2 = 1;
        maxDepth = 8;
        pair<string, int> mv;

        // 対戦
        while (true)
        {
            recieve = client.Recv();        // サーバーからの文字列を受け取る
            result = game::isEnd(recieve); // ゲームの終了判定
            if (result)
                break;             // 終了してたら繰り返し抜ける
            Tree.SetNode(recieve); //受け取った文字列通りにセット
            // Tree.Search();         //探索
            std::thread t1(uct, std::ref(Tree));
            std::thread t2(minmax, std::ref(mv), std::ref(recieve), std::ref(turn));
            t1.join();
            t2.join();
            // printf("Finish Search\n");
            // Tree.PrintStatus();
            if(mv.second > 10000000){
                send = mv.first;
                cout << "SELECT : 紫駒" << endl;
                cout << send << endl;
            }else{
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
