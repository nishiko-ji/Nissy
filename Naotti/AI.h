#pragma once
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

//紫駒
pair<MoveCommandN, int> thinkMove(int depth)
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
	Search searchObj;
	return searchObj.think(bb, pnum, depth);
}

//手を決めるのと、いろんな処理
pair<string, int> solve(int turnCnt, int depth)
{
	pair<string, int> mv;

	//手を決める
	pair<MoveCommandN, int> res = thinkMove(depth);
	MoveCommandN te = res.first;

	mv.first = move(te.y, te.x, te.dir);
	mv.second = res.second;
	return mv;
}

pair<string, int> tsumi(string recv_msg, int turnCnt, int depth)
{
	struct timeval start, end;
	recvBoard(recv_msg);
	gettimeofday(&start, NULL);
	pair<string, int> mv = solve(turnCnt, depth); //思考
	gettimeofday(&end, NULL);
	// printf("time : %f sec\n", (end.tv_sec - start.tv_sec) + 1e-6 * (end.tv_usec - start.tv_usec));
	return mv;
}
