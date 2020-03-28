#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

int dx[] = {-1,1,0,0};
int dy[] = {0,0,-1,1};

char save[30][30];
bool visit[30][30];

int dfs(int x,int y){
	int sum_= 0; visit[x][y] = true;
	for (int i=0;i<4;i++){
		int x_ = x+dx[i];
		int y_ = y+dy[i];
		if(!visit[x_][y_]&&save[x_][y_]=='1')
			sum_+=dfs(x_,y_);
	}
	if(sum_)
		return sum_+1;
	else
		return 1;
}

int main(int argc, char const *argv[])
{
	int N; scanf("%d",&N);
	for (int i=0;i<N;i++){
		char temp; scanf("%c",&temp);
		for (int j=0;j<N;j++)
			scanf("%c",&save[i+1][j+1]);
	}

	for (int i=0;i<N+2;i++)
		visit[i][0]=visit[N+1][0]=visit[0][i]=visit[N+1][i] = true;

	vector<int> res;

	for (int i=0;i<N;i++){
		for (int j=0;j<N;j++){
			if(!visit[i+1][j+1]&&save[i+1][j+1]=='1')
				res.push_back(dfs(i+1,j+1));
		}
	}

	sort(res.begin(),res.end());

	printf("%lu\n",res.size());
	for (int i=0;i<res.size();i++)
		printf("%d\n",res[i]);

	return 0;
}