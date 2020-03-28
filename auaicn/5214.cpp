#include <stdio.h>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

#define HYPER_TUBE (100010)

vector<int> edge[102000];
bool visit[102000];

int main(int argc, char const *argv[])
{
	int V,K,M; scanf("%d %d %d",&V,&K,&M); // (1 ≤ N ≤ 100,000, 1 ≤ K, M ≤ 1000)
	for (int i=0;i<M;i++){
		for (int j=0;j<K;j++){
			int temp; scanf("%d",&temp);
			edge[i+HYPER_TUBE].push_back(temp);
			edge[temp].push_back(HYPER_TUBE+i);
		}

	}
	// 1 to N
	queue<int> q; q.push(1); visit[1] = true;
	int cnt_ = 0;
	bool hy = false;
	while(!q.empty()){
		int pos = q.front(); q.pop();

		if(hy && pos < HYPER_TUBE){
			cnt_++;
			hy^=1;
		}

		if(!hy && pos >= HYPER_TUBE){
			cnt_++;
			hy^=1;
		}

		if(pos == V){
			printf("%d\n",cnt_/2+1);
			return 0;
		}

		for (int i=0;i<edge[pos].size();i++){
			int next = edge[pos][i];
			if(!visit[next]){
				q.push(next);
				visit[next] = true;
			}
		}
	}
	printf("-1\n");
	return 0;
}