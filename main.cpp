#include <iostream>
#include <string>
#include "game.h"

int main()
{
    // サーバ接続用
    string destination; // IPアドレス
    int port;           // ポート番号

    // 対戦管理用
    int n;        // 対戦回数
    int res;      // 対戦結果
    int win = 0;  // 勝った回数
    int draw = 0; // 引き分け回数
    int lose = 0; // 負けた回数

    // 対戦の設定
    cout << "対戦回数 ポート番号 IPアドレス ↓" << endl;
    cin >> n >> port >> destination;

    // AIの設定
    srand((unsigned int)time(NULL));    // ランダムのシードを時間で設定
                                        //
    // 対戦
    for (int i = 0; i < n; i++)
    {
        cout << "(勝ち, 引き分け, 負け) = (" << win << ", " << draw << ", " << lose << ")" << endl;
        res = game::playgame(port, destination); // 対戦
        if (res == game::WON)
            win++; // 勝ち
        if (res == game::DRW)
            draw++; // 引き分け
        if (res == game::LST)
            lose++; // 負け
        sleep(5);
    }

    // 結果出力
    cout << "######result######" << endl;
    cout << "(勝ち, 引き分け, 負け) = (" << win << ", " << draw << ", " << lose << ")" << endl;

    return 0;
}
