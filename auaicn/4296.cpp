#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>

using namespace std;

int dfsn[100010];
bool finished[100010];

int global_cnt = 1;
int scc_num[100010];
int indegree[100010];

vector<pair<int,int> > edges;

vector<int> v[100010];

int num_scc = 0;

stack<int> s;

int scc(int x){

	dfsn[x] = global_cnt++; s.push(x);
	int result = dfsn[x];

	for (int i=0;i<v[x].size();i++){
		int next = v[x][i];
		if(!dfsn[next])
			result = min(result,scc(next));
		else if (!finished[next])
			result = min(result,dfsn[next]);
		else
			continue;
	}

	if(result==dfsn[x]){
		num_scc++;
		while(true){
			int top_ = s.top(); s.pop(); 
			finished[top_] = true;
			scc_num[top_] =  num_scc;
			if(top_==x) 
				break;
		}
	}

	return result;
}

int main(int argc, char const *argv[])
{
	int T; scanf("%d",&T);
	while(T--){
		int N,M; scanf("%d %d",&N,&M);

		//scan
		for (int i=0;i<M;i++){
			int left,right; scanf("%d %d",&left,&right);
			v[left].push_back(right);
			edges.push_back({left,right});
		}

		//solution
		for (int i=0;i<N;i++)
			if(!finished[i+1]){
				scc(i+1);
			}

		for (int i=0;i<edges.size();i++){
			int left = edges[i].first;
			int right = edges[i].second;
			int group_left = scc_num[left];
			int group_right = scc_num[right];
			if(group_right!=group_left)
				indegree[group_right]++;
		}

		int result = 0;
		for (int i=0;i<num_scc;i++)
			if(!indegree[i+1])
				result++;

		printf("%d\n",result);
		
		//initialize
		for (int i=0;i<N;i++){
			dfsn[i+1] = 0;
			finished[i+1] = false;
		}

		for (int i=0;i<N;i++)
			v[i+1].clear();
		for (int i=0;i<num_scc;i++)
			indegree[i+1] = 0;
		edges.clear();

		global_cnt = 1;
		num_scc = 0;

	}
	return 0;
}