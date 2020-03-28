#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

vector<int> v[1010];
bool visit[1010];

void dfs(int x){
	visit[x] = true;
	for (int i=0;i<v[x].size();i++){
		int next = v[x][i];
		if(!visit[next])
			dfs(next);
	}
}

int main(int argc, char const *argv[])
{
	int N,M; scanf("%d %d",&N,&M);
	for (int i=0;i<M;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[left].push_back(right);
		v[right].push_back(left);
	}

	int res = 0;
	for (int i=0;i<N;i++){
		if(!visit[i+1]){
			res++;
			dfs(i+1);
		}
	}
	printf("%d\n",res);
	return 0;
}