#include "Game.h"
#include "Search.h"
#include <vector>
#include <map>
#include <ctime>
#include <algorithm>
#include <cassert>
#include <tuple>
#include <functional>
#include <iostream>
#include <sys/time.h>

using namespace std;
using namespace Game_;

//指し手を決める (Search.hは, (盤面, pnum)が同じなら必ず同じ手を返すアルゴリズムなので、メモ化しても問題なし）
Search searchObj;
clock_t sumThinkTime = 0;
int maxDepth; //探索の深さ(推奨：5〜6）

//紫駒
pair<MoveCommandN, int> thinkPurple()
{
	int i, j;

	string s;
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 6; j++)
		{
			s += board[i][j];
		}
	}
	int pnum = Game_::uNum - Game_::rNum;
	BitBoardN bb;
	bb.toBitBoard(s);
	return searchObj.think(bb, pnum, maxDepth);
}

//手を決める
pair<MoveCommandN, int> thinkMove()
{
	pair<MoveCommandN, int> res;
	res = thinkPurple();
	return res;
}

//手を決めるのと、いろんな処理
pair<string, int> solve(int turnCnt)
{
	pair<string, int> mv;
	//時間計測開始
	clock_t startTime = clock();

	//手を決める
	pair<MoveCommandN, int> res = thinkMove();
	MoveCommandN te = res.first;

	//思考時間
	sumThinkTime += clock() - startTime;

	//表示
	cout << "選択手(" << te.y << ", " << te.x << ", " << te.dir << "), 評価値 = " << res.second << endl;
	mv.first = move(te.y, te.x, te.dir);
	mv.second = res.second;
	// return move(te.y, te.x, te.dir);
	return mv;
}

//駒の初期配置
string initRedName;
void setInitRedName(int allNum = 0, int redNum = 0)
{
	if (allNum == 8)
		return;
	int ransu = rand() % (8 - allNum);
	if (ransu < 4 - redNum)
	{
		initRedName += (char)('A' + allNum);
		setInitRedName(allNum + 1, redNum + 1);
	}
	else
	{
		setInitRedName(allNum + 1, redNum);
	}
}

pair<string, int> tsumi(string recv_msg, int turnCnt)
{
	struct timeval start, end;
	recvBoard(recv_msg);
	gettimeofday(&start, NULL);
	pair<string, int> mv = solve(turnCnt); //思考
	gettimeofday(&end, NULL);
	printf("time : %f sec\n", (end.tv_sec - start.tv_sec) + 1e-6 * (end.tv_usec - start.tv_usec));
	return mv;
}
