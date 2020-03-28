#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int N;
int depth = -1;

int indegree[1010];
int path[1010];

vector<int> v[1010];

int dfs(int now){

	depth++;
	path[depth] = now;

	//path found
	if(depth == N){
		for (int i=0;i<depth;i++)
			printf("%3d",path[i+1]);
		printf("\n");
		depth--; return 1;
	}

	//recursive call
	int res = 0;
	for (int i=0;i<v[now].size();i++){
		int next = v[now][i];
		indegree[next]--;
		if(!indegree[next])
			res+= dfs(next);
		indegree[next]++;
	}
	printf("hello\n");
	depth--; return res;
}

int main(int argc, char const *argv[]) {
	int M; scanf("%d %d",&N,&M);
	// print all possible topological sort 
	// each vertex numbered 1 to N
	// N upto 10E3

	printf("hello\n");

	for (int i=0;i<M;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[left].push_back(right);
		indegree[right]++;
	}

	for (int i=0;i<N;i++){
		v[0].push_back(i+1);
		indegree[i+1]++;
	}

	printf("hello\n");

	printf("%d\n",dfs(0));

	return 0;
}