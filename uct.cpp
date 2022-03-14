#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <time.h>
#include <cassert>
#include <random>

#include "types.h"
#include "method.h"
#include "uct.h"

// linux用
#include <bitset>
#include <cmath>

using namespace std;

// linuxは未定義なので実装
uint64_t __popcnt64(uint64_t data)
{
	bitset<64> bs(data);
	return bs.count();
}

//移動前の位置pposと移動先の位置nposから boardを更新する
void toNextBoard(Board &board, BitBoard ppos, BitBoard npos, bool nowPlayer)
{
	if (Goal(npos, nowPlayer)) //脱出した
		board.escape = true;
	else //脱出してない
		board.escape = false;

	if (nowPlayer == 0)
	{
		if (onPiece(board.myblue, ppos))		//青を動かした
			change(board.myblue, ppos, npos); //駒の移動
		else																//赤を動かした
			change(board.myred, ppos, npos);

		//青駒を取った
		if (onPiece(board.enblue, npos))
		{
			board.enemy ^= npos;
			board.enblue ^= npos;
			board.dead_enblue <<= 1;
			board.kill = true;
		}
		//赤駒を取った
		else if (onPiece(board.enred, npos))
		{
			board.enemy ^= npos;
			board.enred ^= npos;
			board.dead_enred <<= 1;
			board.kill = true;
		}
		else if (onPiece(board.enemy, npos))
		{
			board.enemy ^= npos; //敵駒を取った後の状態に
			board.kill = true;
		}
		else //敵駒を取らなかった
		{
			board.kill = false;
		}
		assert(board.enemy != 0);
	}
	else
	{
		change(board.enemy, ppos, npos);
		if (onPiece(board.enblue, ppos))
			change(board.enblue, ppos, npos);
		if (onPiece(board.enred, ppos))
			change(board.enred, ppos, npos);

		if (onPiece(board.myblue, npos)) //青駒を取られた
		{
			board.myblue ^= npos;
			board.kill = true;
			board.dead_myblue <<= 1;
		}
		else if (onPiece(board.myred, npos)) //赤駒を取られた
		{
			board.myred ^= npos;
			board.kill = true;
			board.dead_myred <<= 1;
		}
		else
		{
			board.kill = false;
		}
	}
}

// すべての合法手後の盤面をvectorに入れる
// 敵駒の色判定はしない
void PossibleNextBoard(vector<Board> &nextpositions, bool nowPlayer, Board board)
{
	BitBoard nowmy = board.myblue | board.myred;

	// getBottomBB()とoffBottomBB()を使って高速に調べるようにしたい
	if (nowPlayer == 0)
	{
		// どの位置の駒を動かすかのfor
		for (BitBoard piecebb = nowmy; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			assert(ppos != 0);
			// 移動4方向を見るfor
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				assert(npos != 0);

				//合法かを見る
				if (!Inside(npos) && !Goal(npos, nowPlayer)) //盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.myred, ppos)) //赤駒が脱出しようとした
					continue;
				if (onPiece(nowmy, npos)) //自身の駒が重なる
					continue;

				board.willplayer = 1;

				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				nextpositions.push_back(nextboard);

			} // for nextbb
		}		// for piecebb
	}			// if nowPlayer == 0
	else
	{
		for (BitBoard piecebb = board.enemy; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (!Inside(npos) && !Goal(npos, nowPlayer)) //盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.enred, ppos)) //赤駒が脱出しようとした
					continue;
				if (onPiece(board.enemy, npos)) //自身の駒が重なる
					continue;

				board.willplayer = 0;

				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				nextpositions.push_back(nextboard);
			}
		}

	} // nowPlayer == 1

	return;
}

//初期化
UCT::UCT(Recieve str)
{
	// ルートノードをつくる
	Nodes.resize(2); // mapの関係で0番目は空にする
	Nodes[1].board = toBoard(str);
	// Nodes[1].value = { 0,0, MAX_VALUE };
	Nodes[1].value = {0, 0};
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;
	pieces = toPieces(str);
}

// 実際の盤面のノードに移動
void UCT::SetNode(Recieve str)
{
	playnum++; //ターン数を一つ進める

	if (!Nodes[playnodenum].nextnodes.empty())
	{
		pieces = toPieces(str);
		Board nowboard = toBoard(str);
		if (nowboard == Nodes[playnodenum].board)
			return;
		for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
		{
			if (Nodes[nextnode].board == nowboard)
			{
				// total_play = Nodes[nextnode].value.play;
				playnodenum = nextnode;
				break;
			}
		}
		if (Nodes[playnodenum].board == nowboard)
			return;
	}
	// 探索木にノードが無ければ構築しなおす
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	// Nodes[1].value = { 0,0, MAX_VALUE };
	Nodes[1].value = {0, 0};
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	pieces = toPieces(str);

	board_index.clear();
}

int livenum(Dead d)
{
	switch (d)
	{
	case 1:
		return 4;
	case 2:
		return 3;
	case 4:
		return 2;
	case 8:
		return 1;
	case 16:
		return 0;
	}
	assert(false);
	return -1;
}

int deadnum(Dead d)
{
	switch (d)
	{
	case 1:
		return 0;
	case 2:
		return 1;
	case 4:
		return 2;
	case 8:
		return 3;
	case 16:
		return 4;
	}
	assert(false);
	return -1;
}

void printEnemy(Board nowboard)
{
	int id;
	int e;
	int r, b;
	int R, B;
	cout << " 1st Player" << endl;
	cout << " 123456" << endl;
	for (int y = 0; y < 6; y++)
	{
		cout << y + 1;
		for (int x = 0; x < 6; x++)
		{
			id = ((y + 1) << 3) + x + 1;
			e = nowboard.enemy >> id;
			r = nowboard.enred >> id;
			b = nowboard.enblue >> id;
			R = nowboard.myred >> id;
			B = nowboard.myblue >> id;
			if (r & 1)
				cout << "r";
			else if (b & 1)
				cout << "b";
			else if (e & 1)
				cout << "e";
			else if (R & 1)
				cout << "R";
			else if (B & 1)
				cout << "B";
			else
				cout << " ";
		}
		cout << endl;
	}
	cout << " 2st Player" << endl;

	cout << "盤面上の敵駒: " << __popcnt64(nowboard.enemy) << endl;
	cout << "生きてる赤駒: " << livenum(nowboard.dead_enred) << endl;
	cout << "生きてる青駒: " << livenum(nowboard.dead_enblue) << endl;
}

void toNextBoardPlayout(Board &board, BitBoard ppos, BitBoard npos, bool nowPlayer)
{
	if (Goal(npos, nowPlayer)) //脱出した
		board.escape = true;
	else //脱出してない
		board.escape = false;

	if (nowPlayer == 0)
	{
		if (onPiece(board.myblue, ppos))		//青を動かした
			change(board.myblue, ppos, npos); //駒の移動
		else																//赤を動かした
			change(board.myred, ppos, npos);

		//青駒を取った
		if (onPiece(board.enblue, npos))
		{
			board.enemy ^= npos;
			board.enblue ^= npos;
			board.dead_enblue <<= 1;
			board.kill = true;
		}
		//赤駒を取った
		else if (onPiece(board.enred, npos))
		{
			board.enemy ^= npos;
			board.enred ^= npos;
			board.dead_enred <<= 1;
			board.kill = true;
		}
		//?駒を取った
		else if (onPiece(board.enemy, npos))
		{
			board.enemy ^= npos;																								 //敵駒を取った後の状態に
																																					 //確率的に色決め
			int bluenum = livenum(board.dead_enblue) - __popcnt64(board.enblue); //わかっていない青の数
			int rednum = livenum(board.dead_enred) - __popcnt64(board.enred);		 //わかっていない赤の数
			int undefinenum = bluenum + rednum;
			// cout << "確率 : " << (double)bluenum / undefinenum << endl;
			// cout << "undefinenum : " << undefinenum << endl;
			// cout << "enemy" << __popcnt64(board.enemy) << endl;
			if (((double)rand() / RAND_MAX) < ((double)bluenum / undefinenum))
			{
				board.dead_enblue <<= 1;
			}
			else
			{
				board.dead_enred <<= 1;
			}
			// printEnemy(board);
			// cout << "(青:赤) = (" << livenum(board.dead_enblue) << ":" << livenum(board.dead_enred) << " )"<< endl;
			board.kill = true;
		}
		else //敵駒を取らなかった
		{
			board.kill = false;
		}
		assert(board.enemy != 0);
	}
	else
	{
		change(board.enemy, ppos, npos);
		if (onPiece(board.enblue, ppos))
			change(board.enblue, ppos, npos);
		if (onPiece(board.enred, ppos))
			change(board.enred, ppos, npos);

		if (onPiece(board.myblue, npos)) //青駒を取られた
		{
			board.myblue ^= npos;
			board.kill = true;
			board.dead_myblue <<= 1;
		}
		else if (onPiece(board.myred, npos)) //赤駒を取られた
		{
			board.myred ^= npos;
			board.kill = true;
			board.dead_myred <<= 1;
		}
		else
		{
			board.kill = false;
		}
	}
}

// UCT内のプレイアウトをする関数
int UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	int bluenum = livenum(nowboard.dead_enblue) - __popcnt64(nowboard.enblue); //わかっていない青の数
	int rednum = livenum(nowboard.dead_enred) - __popcnt64(nowboard.enred);		 //わかっていない赤の数
	int undefinenum = bluenum + rednum;

	int ennum = __popcnt64(nowboard.enemy); //生きている敵の数

	// printEnemy(nowboard);

	//倒されている敵駒を色決めする
	while (ennum < undefinenum)
	{
		int r = rand() % undefinenum;
		if (r < bluenum)
		{
			nowboard.dead_enblue <<= 1;
			bluenum--;
		}
		else
		{
			nowboard.dead_enred <<= 1;
			rednum--;
		}
		undefinenum--;
	}

	while (true)
	{
		// cout << "(青:赤) = (" << livenum(nowboard.dead_enblue) << ":" << livenum(nowboard.dead_enred) << " )"<< endl;

		//勝負がついているか見る
		if (turnnum > MAXPLAY)
		{
			// cout << "Return1!!" << endl;
			return DRAW_VALUE;
		}
		if (nowboard.escape)
		{
			// cout << "Return2!!" << endl;
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
		}
		if (nowboard.dead_myblue == 16 || nowboard.dead_enred == 16)
		{
			// cout << "Return3!!" << endl;
			return LOSE_VALUE;
		}
		if (nowboard.dead_myred == 16 || nowboard.dead_enblue == 16)
		{
			// cout << "Return4!!" << endl;
			return WIN_VALUE;
		}
		// assert(__popcnt64(nowboard.enemy) >= 2);
		// assert(nowboard.enemy == (nowboard.enblue ^ nowboard.enred));

		//変えてみる
		BitBoard ppos = 0, npos = 0;

		//ゴールマスの敵駒の色決め
		if (nowPlayer == 1 && Goal(getNextPosBB(nowboard.enemy), 1))
		{
			BitBoard undefine_enemy = (nowboard.enemy ^ nowboard.enred) ^ nowboard.enblue;
			// if(!Goal(getNextPosBB(nowboard.enred), 1) && !Goal(getNextPosBB(nowboard.enblue), 1)){
			if (Goal(getNextPosBB(undefine_enemy), 1))
			{
				//確率計算
				// int bn = livenum(nowboard.dead_enblue) - __popcnt64(nowboard.enblue); //わかっていない青の数
				int bn = __popcnt64(nowboard.enemy) - __popcnt64(nowboard.enblue); //わかっていない青の数
				int rn = __popcnt64(nowboard.enemy) - __popcnt64(nowboard.enred);	 //わかっていない赤の数
				int un = bn + rn;

				// cout << "----------------------------------------------------------------------------------------------------------" << endl;
				// cout << "確率 : " << (double)rn / un << endl;
				if (((double)rand() / RAND_MAX) < ((double)rn / un))
				{
					// if(true/*確率変数*/){
					if (undefine_enemy & ENGOAL1_MAS)
					{
						nowboard.enred |= ENGOAL1_MAS;
					}
					else if (undefine_enemy & ENGOAL2_MAS)
					{
						nowboard.enred |= ENGOAL2_MAS;
					}
					else
					{
						assert(false);
					}
					undefinenum--;
					rednum--;
				}
				else
				{
					if (undefine_enemy & ENGOAL1_MAS)
					{
						nowboard.enblue |= ENGOAL1_MAS;
					}
					else if (undefine_enemy & ENGOAL2_MAS)
					{
						nowboard.enblue |= ENGOAL2_MAS;
					}
					else
					{
						assert(false);
					}
					undefinenum--;
					bluenum--;
				}
				// printEnemy(nowboard);
			}
			// }
		}

		//ゴールできたらゴールする
		if (nowPlayer == 0 && Goal(getNextPosBB(nowboard.myblue), 0))
		{
			ppos = getNextPosBB(MYGOAL) & nowboard.myblue;
			// assert(__popcnt64(ppos) == 1);	//探索木の構造上、両方の手前にあることがある
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & MYGOAL;
			assert(__popcnt64(npos) == 1);
		}
		else if (nowPlayer == 1 && Goal(getNextPosBB(nowboard.enblue), 1))
		{
			// cout<<"-------------------------------------------------------------------------------------------------" << endl;
			ppos = getNextPosBB(ENGOAL) & nowboard.enblue;
			// assert(__popcnt64(ppos) == 1);
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//通常の動き
		else
		{
			ppos = (nowPlayer == 0 ? (nowboard.myblue | nowboard.myred) : nowboard.enemy);
			npos = Inside(getNextPosBB(ppos) & ~ppos); //移動先候補を抽出
			assert(ppos != 0);
			assert(npos != 0);
			//移動先をランダムに選択
			int r = rand() % __popcnt64(npos);
			while (r--)
				offBottomBB(npos);
			npos = getBottomBB(npos);
			//選択された移動先に行ける駒の中でランダムに選択
			ppos &= getNextPosBB(npos);
			assert(ppos != 0);
			r = rand() % __popcnt64(ppos);
			while (r--)
				offBottomBB(ppos);
			ppos = getBottomBB(ppos);
		}
		//決めた行動をboardに変換する
		toNextBoardPlayout(nowboard, ppos, npos, nowPlayer);
		//

		//ターンを進める
		nowPlayer = !nowPlayer;
		turnnum++;
	} // while(true)
}

// UCT探索をする
void UCT::Search()
{
	//事前にSetNode(str)をしておく
	NodeNum search_node = playnodenum;
	vector<NodeNum> usenode(1, playnodenum);
	unordered_map<NodeNum, bool> used;
	bool nowPlayer = 0; // 0:自分  1:相手
	int turnnum = playnum;

	assert(Nodes[playnodenum].board.willplayer == nowPlayer);

	clock_t start_time = clock();
	int i = 0;
	// while (i < 30000) // UCTのループ
	while (clock() - start_time < 9.0 * CLOCKS_PER_SEC) // UCTのループ
	{
		while (!Nodes[search_node].nextnodes.empty()) //木の端まで見る
		{
			NodeNum choice_nodenum = -1;
			double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
			assert(Nodes[search_node].board.willplayer == nowPlayer);
			for (NodeNum nextnode : Nodes[search_node].nextnodes)
			{
				assert(Nodes[nextnode].board.willplayer != nowPlayer);
				if (used[nextnode]) // 2度同じ盤面は見ない
					continue;
				// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
				// 相手番でプレイアウト回数を考慮できていなかったのを修正
				if (Nodes[nextnode].value.play == 0)
				{
					choice_nodenum = nextnode;
					break;
				}
				// if ((nowPlayer == 0 ? chmax(maxvalue, Nodes[nextnode].value.comp) : chmin(minvalue, Nodes[nextnode].value.comp)))
				if ((nowPlayer == 0 ? chmax(maxvalue, compare(Nodes[nextnode].value.win, Nodes[nextnode].value.play)) : chmax(maxvalue, compare(-Nodes[nextnode].value.win, Nodes[nextnode].value.play))))
				{
					choice_nodenum = nextnode;
				}
			}
			if (choice_nodenum == -1) //見たことある盤面しかなければプレイアウト
			{
				break;
			}

			turnnum++;
			used[choice_nodenum] = 1;
			search_node = choice_nodenum;
			nowPlayer = !nowPlayer;
			usenode.push_back(search_node);
		} // while(!empty())

		if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1) //展開
		{
			// 打てる手の候補を抽出する
			vector<Board> nextboards;
			PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board);
			int nextboards_size = nextboards.size();
			assert(nextboards_size > 0);

			// Nodesの拡張
			for (const Board &nextboard : nextboards)
			{
				assert(nowPlayer == Nodes[search_node].board.willplayer);
				assert(nextboard.willplayer != nowPlayer);
				assert(Nodes[search_node].board.willplayer != nextboard.willplayer);
				if (board_index[nextboard] != 0) //ハッシュが設定済み（探索済みのノード）は繋げるだけ
				{
					assert(Nodes[search_node].board.willplayer != Nodes[board_index[nextboard]].board.willplayer);
					Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
					continue;
				}
				board_index[nextboard] = nodenum; //新たにハッシュを設定する

				Node nextnode;
				nextnode.board = nextboard;
				nextnode.nextnodes.clear();

				//勝負がついているか
				nextnode.finish = (__popcnt64(nextnode.board.enemy) < 2 ||
													 nextnode.board.dead_enblue == 16 ||
													 nextnode.board.dead_enred == 16 ||
													 nextnode.board.dead_myblue == 16 ||
													 nextnode.board.dead_myred == 16 ||
													 nextnode.board.escape);

				Nodes.push_back(nextnode);
				assert(Nodes[search_node].board.willplayer != Nodes[nodenum].board.willplayer);
				Nodes[search_node].nextnodes.push_back(nodenum);
				assert(Nodes[nodenum] == nextnode);
				nodenum++;
			} // for nextmoves_size

			assert(!Nodes[search_node].nextnodes.empty());
			search_node = Nodes[search_node].nextnodes[0];
			nowPlayer = !nowPlayer;
			usenode.push_back(search_node);
		} // if 展開

		//プレイアウトとバックプロパゲーション
		{
			// playout();
			// cout<<__popcnt64(Nodes[search_node].board.enemy) << endl;
			int reward = playout(nowPlayer, turnnum, Nodes[search_node].board);
			i++;

			// backpropagation();
			// mtx.lock();
			total_play++;
			for (NodeNum node : usenode)
			{
				Nodes[node].value.play++;
				Nodes[node].value.win += reward;
				// UpdateBoardValue(Nodes[node].value);
			}
			// mtx.unlock();
			//探索情報をリセット
			usenode.resize(1, playnodenum);
			search_node = playnodenum;
			nowPlayer = 0;
			turnnum = playnum;
			used.clear();
		} // else
	}		// while(true)	UCTのループ
} // Search()

//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
MoveCommand UCT::toMoveCommand(NodeNum from, NodeNum to)
{
	cout << "from:" << from << " to:" << to << endl;
	assert(from != to);
	assert(Nodes[from].board.willplayer != Nodes[to].board.willplayer);
	MoveCommand move;
	BitBoard from_bb = Nodes[from].board.myblue | Nodes[from].board.myred;
	BitBoard to_bb = Nodes[to].board.myblue | Nodes[to].board.myred;
	assert(from_bb != to_bb);
	BitBoard ppos = from_bb & ~to_bb;
	BitBoard npos = ~from_bb & to_bb;
	assert(ppos != 0);
	assert(__popcnt64(ppos) == 1);
	assert(npos != 0);
	assert(__popcnt64(npos) == 1);
	Point ppoint = toPoint(ppos);
	Point npoint = toPoint(npos);
	move.xy = ppoint;
	move.dir = toDir(ppoint, npoint);
	return move;
}

// rootノードからどの手を選んで指すか。ノードの添え字を返す
NodeNum UCT::Choice()
{
	assert(playnodenum < Nodes.size());
	assert(!Nodes[playnodenum].nextnodes.empty());
	NodeNum choice_nodenum = -1;
	double maxvalue = -MAX_VALUE;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// 自分の手番⇒大きいのを選ぶ
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(choice_nodenum != -1);
	assert(playnodenum != choice_nodenum);
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play << endl;
	return choice_nodenum;
}

//ノードを遷移して、サーバーに送る文字列を返す（送ってはない）
Send UCT::MoveNode(NodeNum move_nodenum)
{
	assert(playnodenum != move_nodenum);
	MoveCommand move = toMoveCommand(playnodenum, move_nodenum);
	Send send = toSend(move, pieces);
	pieces.pos[getPieceNum(pieces, move.xy)] = nextpos(move);
	playnodenum = move_nodenum;
	return send;
}

void UCT::PrintStatus()
{
	printf("Turn:%d\n", playnum);
	printf("Total Playout Times : %d\n", total_play);
	printf("Node Num : %d\n", nodenum);
}

string UCT::getStatus(NodeNum node)
{
	Board nowboard = Nodes[node].board;
	string status;
	status = "#####################################\n";
	status += "Turn: " + to_string(playnum) + "\n";
	status += "Total Playout Times: " + to_string(total_play) + "\n";
	status += "Node Num: " + to_string(nodenum) + "\n\n";
	status += " enemySide\n";
	status += " 123456\n";
	for (int y = 0; y < 6; y++)
	{
		status += to_string(y + 1);
		for (int x = 0; x < 6; x++)
		{
			int id = ((y + 1) << 3) + x + 1;
			if ((nowboard.myred >> id) & 1)
				status += "R";
			else if ((nowboard.myblue >> id) & 1)
				status += "B";
			else if ((nowboard.enred >> id) & 1)
				status += "r";
			else if ((nowboard.enblue >> id) & 1)
				status += "b";
			else if ((nowboard.enemy >> id) & 1)
				status += "e";
			else
				status += " ";
		}
		status += "\n";
	}
	status += " mySide\n";
	status += "dead enemy red:  " + to_string(deadnum(nowboard.dead_enred)) + "\n";
	status += "dead enemy blue: " + to_string(deadnum(nowboard.dead_enblue)) + "\n";
	status += "Value:" + to_string(Nodes[node].value.win) + " Play:" + to_string(Nodes[node].value.play) + "\n";

	return status;
}
