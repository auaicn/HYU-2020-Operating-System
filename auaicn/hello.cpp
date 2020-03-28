#include <iostream>
#include <stdio.h>
#include <queue>
using namespace std;

typedef struct status {
	status(int x,int y, int dir) {
		this->x = x;
		this->y = y;
		this->dir = dir;
	}
	int x;
	int y;
	int dir;
};

bool visit[110][110][5];

int drt[5][2] = { {0,0},{ 3,4 } ,{ 3,4 },{ 1,2 } ,{ 1,2 } };
int straight[5][2] = { {0,0},{0,1},{0,-1},{1,0},{-1,0} };
int board[110][110];

int main() {

	int N, M; scanf("%d %d", &N, &M);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			scanf("%d", &board[i + 1][j + 1]);
		}
	}

	for (int i = 0; i < N + 2; i++) {
		board[i][0] = board[i][M + 1] = 1;
		for (int j = 0; j < 4; j++)
			visit[i][M + 1][j + 1] = visit[i][0][j + 1] = true;
	}

	for (int i = 0; i < M + 2; i++) {
		board[0][i] = board[N + 1][i] = 1;
		for (int j = 0; j < 4; j++)
			visit[0][i][j + 1] = visit[N + 1][i][j + 1] = true;
	}

	int x_start, y_start, dir_start; scanf("%d %d %d", &x_start, &y_start, &dir_start);
	int x_end, y_end, dir_end; scanf("%d %d %d", &x_end, &y_end, &dir_end);
	
	queue<pair<status, int>> qu;
	qu.push({ status(x_start,y_start,dir_start),0 });
	visit[x_start][y_start][dir_start] = true;

	while (!qu.empty()) {

		pair<status, int> pos = qu.front(); qu.pop();
		status now = pos.first;

		// check finish
		if (now.x == x_end && now.y == y_end&& now.dir == dir_end) {
			cout << pos.second << endl;
			return 0;
		}

		// rotation
		if (!visit[now.x][now.y][drt[now.dir][0]]) {
			// not visited
			visit[now.x][now.y][drt[now.dir][0]] = true;
			qu.push({ status(now.x,now.y,drt[now.dir][0]), pos.second + 1 });


		}
		if (!visit[now.x][now.y][drt[now.dir][1]]) {
			// not visited
			visit[now.x][now.y][drt[now.dir][1]] = true;
			qu.push({ status(now.x,now.y,drt[now.dir][1]), pos.second + 1 });
		}

		// straight
		bool blocked = false;

		// Go 1
		int dx = straight[now.dir][0];
		int dy = straight[now.dir][1];

		int x = now.x + dx;
		int y = now.y + dy;

		if (board[x][y])
			blocked = true;

		if (!blocked) {
			if (!visit[x][y][now.dir]) {
				// not visited
				visit[x][y][now.dir] = true;
				qu.push({ status(x,y,now.dir),pos.second + 1 });
			}
		}

		x += dx;
		y += dy;

		if (!blocked&&board[x][y])
			blocked = true;
		
		if (!blocked) {
			if (!visit[x][y][now.dir]) {
				// not visited
				visit[x][y][now.dir] = true;
				qu.push({ status(x,y,now.dir),pos.second + 1 });
			}
		}

		x += dx;
		y += dy;

		if (!blocked&&board[x][y])
			blocked = true;
		if (!blocked) {
			if (!visit[x][y][now.dir]) {
				// not visited
				visit[x][y][now.dir] = true;
				qu.push({ status(x,y,now.dir),pos.second + 1 });
			}
		}

	}

	return 0;
}
