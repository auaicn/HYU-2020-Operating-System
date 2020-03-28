#include <stdio.h>
#include <iostream>
#include <stack>
#include <vector>

using namespace std;

int dfsn[100010];
bool finished[100010];

int global_dfs_num = 1;

vector<int> v[100010];

stack<int> s;

int scc(int x){
	dfsn[x] =global_dfs_num++; s.push(x);

	int result = 



}

int main(int argc, char const *argv[])
{
	int V,E; scanf("%d %d",&V,&E);
	for (int i=0;i<E;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[left].push_back(right);
	}

	for (int i=0;i<N;i++)
		if(!finished[i+1])
			scc(i+1);

	return 0;
}
