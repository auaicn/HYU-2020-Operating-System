#include <stdio.h>
#include <queue>
#include <iostream>

using namespace std;

#define CORNER (-1)

int board[1010][1010];
int res[1010][1010][2];

int dx[]={-1,1,0,0};
int dy[]={0,0,-1,1};

int min(int x, int y){
	return x<y?x:y;
}

int main(int argc, char const *argv[])
{
	int N,M; scanf("%d %d",&N,&M);
	for (int i=0;i<N;i++){
		char temp; scanf("%c",&temp);
		for (int j=0;j<M;j++){
			scanf("%c",&temp);
			if(temp=='0')
				board[i+1][j+1] = 0;
			else
				board[i+1][j+1] = 1;
		}
	}

	for (int i=0;i<N+2;i++)
		board[i][0] = board[i][M+1] = CORNER;
	for (int i=0;i<M+2;i++)
		board[0][i] = board[N+1][i] = CORNER;

	queue<pair<pair<int,int>,int> > q;
	if(board[1][1]){
		q.push({{1,1},1});
		res[1][1][1] = 1;
		res[1][1][0] = 1;
	}
	else{
		q.push({{1,1},0});
		res[1][1][0] = 1;
		res[1][1][1] = 1;
	}

	while(!q.empty()){
		pair<pair<int,int>,int> pos = q.front(); q.pop();
		int x = pos.first.first;
		int y = pos.first.second;
		int crashed = pos.second;
		int cost = res[x][y][crashed];
		//printf("now %3d%3d\n",x,y);
		if(crashed){
			// crashed == 1
			for (int i=0;i<4;i++){
				int x_ = x+dx[i];
				int y_ = y+dy[i];
				if(board[x_][y_]==CORNER) // corner case handle
					continue;
				if(board[x_][y_]) // now we cannot crash any wall
					continue;
				if(res[x_][y_][1]) //already visited 
					continue; // actually it cannot be executed
				else{
					if(board[x_][y_]==0){
						q.push({{x_,y_},1}); 
						res[x_][y_][1] = cost+1;
					}
				}
			}
		}else{
			// not crahsed. we can crash one wall
			// crashed == 0
			for (int i=0;i<4;i++){
				int x_ = x+dx[i];
				int y_ = y+dy[i];
				if(board[x_][y_]==CORNER)
					continue;
				if(board[x_][y_]){
					// wall
					if(res[x_][y_][1])
						// visited then continue
						continue;
					else{
						// now push with second element '1'
						q.push({{x_,y_},1});
						res[x_][y_][1] = cost+1;
					}
				}else{
					if(res[x_][y_][0])
						//visited then continue
						continue;
					else{
						// not visited then push to the queue
						q.push({{x_,y_},0});
						res[x_][y_][0] = cost+1;
					}
				}
			}
		}
	}

	/*
		printf("\n");

	for (int i=0;i<N;i++){
		for (int j=0;j<M;j++){
			printf("%3d",res[i+1][j+1][0]);
		}
		printf("\n");
	}
		printf("\n");

	for (int i=0;i<N;i++){
		for (int j=0;j<M;j++){
			printf("%3d",res[i+1][j+1][1]);
		}
		printf("\n");
	}
		printf("\n");
		*/

	//print result
	if(res[N][M][0]&&res[N][M][1]){
		printf("%d\n",min(res[N][M][0],res[N][M][1]));
	}else if(res[N][M][0]){
		printf("%d\n",res[N][M][0]);
	}else if(res[N][M][1]){
		printf("%d\n",res[N][M][1]);
	}else printf("-1\n");
	
	return 0;

}