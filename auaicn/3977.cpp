#include <stdio.h>
#include <iostream>
#include <stack>
#include <vector>
#include <algorithm>

using namespace std;

int dfsn[100010]; int global_dfsn_cnt =1;
bool finished[100010];

vector<int> v[100010];
vector<pair<int,int> > edges;
vector<int> connection_after_scc[100010];
vector<int> group_component[100010];

int indegree[100010];
int group[100010];
int num_scc;

bool visit[100010];

stack<int> s;

int scc(int x){
	
	dfsn[x] = global_dfsn_cnt++; s.push(x);

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
			group[top_] = num_scc; finished[top_] = true;
			group_component[num_scc].push_back(top_);
			if(top_==x)
				break;
		}
	}

	return result;
}

int dfs(int x){
	visit[x] = true;
	int res = 1;
	for (int i=0;i<connection_after_scc[x].size();i++){
		int next = connection_after_scc[x][i];
		if(!visit[next])
			res+=dfs(next);
	}
	return res;
}

int main(int argc, char const *argv[])
{
	int T; scanf("%d",&T);
	while(T--){
		int N,M; scanf("%d %d",&N,&M);
		for (int i=0;i<M;i++){
			int left,right; scanf("%d %d",&left,&right); left++; right++;
			v[left].push_back(right);
			edges.push_back({left,right});
		}

		//solution
		for (int i=0;i<N;i++)
			if(!finished[i+1])
				scc(i+1);

		for (int i=0;i<edges.size();i++){
			int left = edges[i].first;
			int right = edges[i].second;
			int group_left = group[left];
			int group_right = group[right];
			if(group_left==group_right)
				continue;
			else{
				connection_after_scc[group_left].push_back(group_right);
				indegree[group_right]++;
			}
		}

		/*
		for (int i=0;i<num_scc;i++){
			printf("%d : ",i+1);
			for (int j=0;j<group_component[i+1].size();j++)
				printf("%3d",group_component[i+1][j]);
			printf("\n");
		}
		*/

		int S = 0;
		for (int i=0;i<num_scc;i++){
			if(!indegree[i+1]){
				if(S){
					S=0;
					break;
				}else
					S = i+1;
			}
		}


		if(!S){
			printf("Confused\n");
		}
		else{
			int temp = dfs(S);
		 	if(temp==num_scc){
		 		vector<int> res= group_component[S];
		 		sort(res.begin(),res.end());
		 		for (int i=0;i<res.size();i++)
		 			printf("%d\n",res[i]-1);
		 	}else{
		 		printf("Confused\n");
		 	}
		}

		printf("\n");

		//init
		for (int i=0;i<num_scc;i++){
			visit[i+1] = false;
			indegree[i+1] = 0;
			group_component[i+1].clear();
			connection_after_scc[i+1].clear();
		}

		num_scc = 0;
		global_dfsn_cnt = 1;

		edges.clear();

		for (int i=0;i<N;i++){
			finished[i+1] = false;
			group[i+1] = 0;
			dfsn[i+1] = 0;
			v[i+1].clear();
		}

	}
	return 0;
}
