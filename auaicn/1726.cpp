#include <stdio.h>
#include <iostream>
#include <queue>

using namespace std;

int dx[] = {0,0,-1,1};
int dy[] = {-1,1,0,0};

bool board[110][110];
int visit[110][110][4];

int main(int argc, char const *argv[])
{

	int N,M; scanf("%d %d",&N,&M);
	for (int i=0;i<N;i++)
		for (int j=0;j<M;j++){
			// 0 then we can go.
			int temp; scanf("%d",&temp);
			board[i+1][j+1] = !temp;
		}

	for (int k=0;k<4;k++){
		for (int i=0;i<N+2;i++){
			visit[i][0][k] = visit[i][M+1][k] = 1;
		}
		for (int i=0;i<M+2;i++){
			visit[0][i][k] = visit[N+1][i][k] = 1;
		}
	}

	int x_start,y_start,dir_start; scanf("%d %d %d",&x_start,&y_start,&dir_start); dir_start = dir_start%2?dir_start:dir_start-2;
	int x_end,y_end,dir_end; scanf("%d %d %d",&x_end,&y_end,&dir_end); dir_end = dir_end%2?dir_end:dir_end-2;
	//printf("__%d %d\n",dir_start,dir_end);

	queue<pair<pair<int,int>,int> > q;

	q.push({{x_start,y_start},dir_start}); visit[x_start][y_start][dir_start] = 1;

	while(!q.empty()){
		pair<pair<int,int>,int>  pos = q.front(); q.pop();
		int x = pos.first.first;
		int y = pos.first.second;
		int dir = pos.second;
		int res = visit[x][y][dir];
		//printf("%d %d %d %d\n",x,y,dir,res);

		if(x==x_end&&y==y_end&&dir==dir_end){
			printf("%d\n",res-1);
			return 0;
		}

		int dir_temp = dir; if(dir_temp%2) dir_temp--;
		int dir1 = (dir_temp+2)%4;
		int dir2 = (dir_temp+2)%4+1;
		//printf("%d %d %d %d\n",dir,dir_temp,dir1,dir2);

		if(!visit[x][y][dir1]){
			//printf("left\n");
			q.push({{x,y},dir1}); visit[x][y][dir1] = res+1;
		}

		if(!visit[x][y][dir2]){
			//printf("right\n");
			q.push({{x,y},dir2}); visit[x][y][dir2] = res+1;
		}

		for (int i=0;i<3;i++){
			int x_ = x+(i+1)*dx[dir];
			int y_ = y+(i+1)*dy[dir];
			//printf("%d %d\n",x_,y_);
			if(!board[x_][y_])
				break;
			else if(!visit[x_][y_][dir]){
				q.push({{x_,y_},dir}); visit[x_][y_][dir] = res+1;
			}
		}
	}

	return 0;
}



