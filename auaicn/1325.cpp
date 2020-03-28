#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

vector<int> v[10010];
bool visit[10010];
int res[10010];

int max(int x, int y){
	return x>y?x:y;
}

int dfs(int now){
	printf("10\n");
	if(res[now])
		return res[now];

	visit[now] = true;
	int sum_ = 0;
	for (int i=0;i<v[now].size();i++){
		int next = v[now][i];
		if(!visit[next])
			sum_+=dfs(next);
	}

	if(sum_==0){
		res[now] = 1;
		return 1;
	}

	res[now] = sum_+1;
	return sum_+1;

}

int main(int argc, char const *argv[])
{
	int N,M; scanf("%d %d",&N,&M);
	for (int i=0;i<M;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[right].push_back(left);
	}

	for (int i=0;i<N;i++)
		if(!visit[i+1])
			dfs(i+1);

	// 
	int max_ = -1;
	vector<int> result;
	for (int i=0;i<N;i++){
		if(max_<res[i+1]){
			max_ = res[i+1];
			result.clear();
			result.push_back(i+1);
		}else if(max_>res[i+1])
			continue;
		else{
			result.push_back(i+1);
		}
	}
	//printf("%d\n",max_);
	for (int i=0;i<result.size();i++){
		printf("%d ",result[i]);
	}
	printf("\n");
	return 0;
}