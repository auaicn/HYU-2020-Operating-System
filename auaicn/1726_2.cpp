#include <stdio.h>
#include <iostream>
#include <queue>

using namespace std;

int dx[] = {0,0,0,1,-1};
int dy[] = {0,1,-1,0,0};

bool board[110][110];
int visit[110][110][5];

int main(int argc, char const *argv[])
{

	int N,M; scanf("%d %d",&N,&M);
	for (int i=0;i<N;i++)
		for (int j=0;j<M;j++){
			int temp; scanf("%d",&temp);
			board[i+1][j+1] = temp? false: true;
		}

	for (int k=0;k<5;k++){
		for (int i=0;i<N+2;i++)
			visit[i][0][k] = visit[i][M+1][k] = 1;
		for (int i=0;i<M+2;i++)
			visit[0][i][k] = visit[N+1][i][k] = 1;
	}

	int x_start,y_start,dir_start; scanf("%d %d %d",&x_start,&y_start,&dir_start);
	int x_end,y_end,dir_end; scanf("%d %d %d",&x_end,&y_end,&dir_end);

	if(x_start==x_end&&y_start==y_end&&dir_start==dir_end){
		printf("0\n");
		return 0;
	}

	queue<pair<pair<int,int>,int> > q;

	q.push({{x_start,y_start},dir_start}); visit[x_start][y_start][dir_start] = 1;

	while(!q.empty()){
		pair<pair<int,int>,int>  pos = q.front(); q.pop();
		int x = pos.first.first;
		int y = pos.first.second;
		int dir = pos.second;
		int res = visit[x][y][dir];

		if(x==x_end&&y==y_end&&dir==dir_end){
			printf("%d\n",res-1);
			return 0;
		}

		int dir1,dir2;
		if(dir>=3){dir1 = 1; dir2 = 2;}
		if(dir<3){dir1 = 3; dir2 = 4;}

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
			else if(visit[x_][y_][dir])
				break;
			else{
				q.push({{x_,y_},dir}); visit[x_][y_][dir] = res+1;
			}
		}
	}

	return 0;
}
