//ガイスターの探索
//不完全情報部分, 完全情報ガイスターの2段階からなる。
// i行j列目(i>=0, j>=0)をマスi * 6 + jとおく。先手の脱出口はマス0, 5。
// cornerId…左上0, 右上1, 左下2, 右下3
#pragma once
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <time.h>
#include "BitBoard.h"
#include "Game.h"
using namespace std;

class Search
{
	int kiki[36 * 5]; // kiki[i * 5 + j] = マスiから行けるj番目のマスの番号. (なければ-1)
	int maxDepth;			// maxDepth    = 探索の深さの最大値

public:
	int INF; // INF         = 評価値の上限 (下限は-INF以上)

	Search() //ゲームが始まる前の処理
	{
		INF = 100000000;
		int y, x, dir, i, j;
		int dy[4] = {-1, 0, 1, 0};
		int dx[4] = {0, 1, 0, -1};

		for (i = 0; i < 180; i++)
			kiki[i] = -1;
		for (y = 0; y < 6; y++)
		{
			for (x = 0; x < 6; x++)
			{
				i = y * 6 + x;
				j = 0;
				for (dir = 0; dir < 4; dir++)
				{
					int ny = y + dy[dir];
					int nx = x + dx[dir];
					if (0 <= ny && ny < 6 && 0 <= nx && nx < 6)
					{
						kiki[5 * i + j] = ny * 6 + nx;
						j++;
					}
				}
			}
		}
	}

	// board[i] = マスiの状態. ([i / 6]行, i % 6列目）
	//敵の駒をpnum個以下にすると、敵が勝つ可能性あり
	pair<MoveCommandN, int> think(BitBoardN bb, int pnum, int maxDepth)
	{
		//探索の設定
		this->pnum = pnum;
		this->maxDepth = maxDepth;

		// 1手で脱出できるか？
		MoveCommandN escapeTe = bb.getEscapeCommand(0);
		if (escapeTe.y >= 0)
			return pair<MoveCommandN, int>(escapeTe, INF);

		// 時間を取得してnegamaxの引数に与える
		time_t st = time(NULL);

		//相手の駒を紫駒とした完全情報探索 (深さ0探索時に最善手を格納）
		int eval = negamax(bb, 0, -INF - 1, INF + 1, st, 8);
		// int eval = negamax(bb, 0, -INF - 1, INF + 1);
		// return pair<MoveCommandN, int>(bestMove, eval);
		if (eval == 0)
		{
			// printf("深さ %d 読みきれなかった。。。\n", maxDepth);
			return pair<MoveCommandN, int>(noMove, eval);
		}
		else
		{
			return pair<MoveCommandN, int>(bestMove, eval);
		}
	}

private:
	int pnum;							 //紫駒がpnum個以下になったら敵の勝ち
	MoveCommandN bestMove; //深さ0（R,Bを動かす手番）における最良手
	MoveCommandN noMove;

	//探索部分 (自分必勝：INF, 自分必負-INF), 戻り値が(alpha, beta)の範囲を超えたら適当に返す
	int negamax(BitBoardN bb, int depth, int alpha, int beta, time_t st, int endtime)
	// int negamax(BitBoardN bb, int depth, int alpha, int beta)
	{
		int player = depth % 2;
		int winPlayer = bb.getWinPlayer(player, pnum);
		if (winPlayer <= 1)
			return player == winPlayer ? INF - depth : -INF + depth;
		if (depth == maxDepth)
			return bb.evaluate(player);

		//時間を取得して引数の時間と比較、9秒経ってたらreturn " ", 0 を返す
		if (time(NULL) - st > endtime)
		{
			// printf("time : %f\n", time(NULL) - st);
			// printf("終了しなかったよ\n");
			return 0;
		}

		int from[32], to[32];
		int moveNum = bb.makeMoves(player, kiki, from, to);

		for (int i = 0; i < moveNum; i++)
		{
			BitBoardN bbTmp = bb;
			bb.move(from[i], to[i]);
			int res = -negamax(bb, depth + 1, -beta, -alpha, st, 8);
			// int res = -negamax(bb, depth + 1, -beta, -alpha);
			if (alpha < res)
			{
				alpha = res;
				if (depth == 0)
				{
					bestMove = MoveCommandN::parse(from[i], to[i]);
				}
			}
			if (alpha >= beta)
			{
				return beta;
			} //βcut
			bb = bbTmp;
		}

		return alpha;
	}
};
