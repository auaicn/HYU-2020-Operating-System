#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

vector<int> v[1010];

bool visit[1010];

void dfs(int x){
	visit[x] = true; printf("%d ",x);
	for (int i=0;i<v[x].size();i++){
		int next = v[x][i];
		if(!visit[next])
			dfs(next);
	}
}

int main(int argc, char const *argv[])
{
	int N,M,S; scanf("%d %d %d",&N,&M,&S);
	for (int i=0;i<M;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[left].push_back(right);
		v[right].push_back(left);
	}

	for (int i=0;i<N;i++)
		sort(v[i+1].begin(),v[i+1].end());

	//dfs
	dfs(S); printf("\n");

	for (int i=0;i<N;i++)
		visit[i+1] = false;

	queue<int> q;
	q.push(S); visit[S] = true;
	while(!q.empty()){
		int pos = q.front(); q.pop(); printf("%d ",pos);
		for (int i=0;i<v[pos].size();i++){
			int next = v[pos][i];
			if(!visit[next]){
				q.push(next); visit[next]= true;
			}
		}
	}
	printf("\n");

	return 0;
}