#include <stdio.h>
#include <iostream>
#include <vector>
#include <stack>

using namespace std;

vector<int> v[500010];

int global_dfsn_cnt = 1;
int dfsn[500010]; bool finished[500010];

int num_scc;

int save[500010];
int sum[500010];
int group[500010];

int visit[500010];
bool restaurant[500010];
bool group_restaurnt[500010];

vector<pair<int,int> > edges;

stack<int> s;

vector<int> connection[500010];

int min(int x, int y){
	return x<y?x:y;
}

int scc(int x){
	
	dfsn[x] = global_dfsn_cnt++; s.push(x);
	int result = dfsn[x]; 

	for (int i=0;i<v[x].size();i++){
		int next = v[x][i];
		if(!dfsn[next]){
			result = min(result,scc(next));
		}else if(!finished[next]){
			result = min(result,dfsn[next]);
		}
	}

	if(result==dfsn[x]){
		num_scc++; 
		while(true){
			int top_ = s.top(); s.pop(); finished[top_] = true;
			group[top_] = num_scc;
			sum[num_scc]+=save[top_];
			if(restaurant[top_])
				group_restaurnt[num_scc] = true;
			if(top_==x)
				break;
		}
	}

	return result;
}

int dfs(int x){
	if(visit[x]>0)
		return visit[x];
	if(visit[x]<0)
		return 0;

	int to_return = 0;
	for (int i=0;i<connection[x].size();i++){
		int next = connection[x][i];
		to_return = max(to_return,dfs(next));
	}
	if(!to_return){
		if(!group_restaurnt[x]){
			visit[x] = -1;
			return 0;
		}else{
			visit[x] = sum[x];
			return visit[x];
		}
	}else{
		to_return+=sum[x];
		visit[x] = to_return;
		return to_return;
	}

}

int main(int argc, char const *argv[])
{
	//scan
	int N,M; scanf("%d %d",&N,&M);
	for (int i=0;i<M;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[left].push_back(right);
		edges.push_back({left,right});
	}

	for (int i=0;i<N;i++)
		scanf("%d",&save[i+1]);

	int S,P; scanf("%d %d",&S,&P);

	for (int i=0;i<P;i++){
		int temp; scanf("%d",&temp);
		restaurant[temp] = true;
	}

	//solution

	for (int i=0;i<N;i++)
		if(!finished[i+1])
			scc(i+1);

	/*
	printf("num scc : %d\n",num_scc);

	for (int i=0;i<num_scc;i++)
		printf("%3d",group_restaurnt[i+1]);
	printf("\n");

	for (int i=0;i<num_scc;i++)
		printf("%3d",sum[i+1]);
	printf("\n");
	*/

	for (int i=0;i<edges.size();i++){
		pair<int,int> now = edges[i];
		int left = now.first;
		int right = now.second;
		int group_left = group[left];
		int group_right = group[right];
		if(group_left==group_right)
			continue;
		else{
			connection[group_left].push_back(group_right);
			//printf("%d to %d\n",group_left,group_right);
		}
	}
	edges.clear();

	// print result
	printf("%d\n",dfs(group[S]));

	return 0;
}

/*
6 5
1 3
2 3
3 4
3 5
5 6
1
3
2
4
8
7
1 4
1 3 4 5
*/







