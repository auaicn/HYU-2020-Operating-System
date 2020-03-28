#include <stdio.h>
#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;


int dfsn[10010];
bool finished[10010];

stack<int> s;

vector<int> v[10010];
vector<vector<int> > res;

int global_cnt = 1;

int min(int x, int y){
	return x<y?x:y;
}

int scc(int x){
	dfsn[x] = global_cnt++;	s.push(x);
	int result = dfsn[x];

	for (int i=0;i<v[x].size();i++){
		int next = v[x][i];
		if(!dfsn[next]){
			// next not visited then recursive call
			result = min(result,scc(next));
		}else if(!finished[next]){
			result = min(result,dfsn[next]);
			// reverse edge
		}else
			continue;
	}
	// no reverse edge found
	if(result==dfsn[x]){
		vector<int> tmp;
		while(true){
			int top_ = s.top(); s.pop(); finished[top_] = true;
			tmp.push_back(top_);
			if(top_==x)
				break;
		}
		sort(tmp.begin(),tmp.end());
		res.push_back(tmp);
	}
	//dfsn[x] = result; // not needed.
	return result;
}

int main(int argc, char const *argv[])
{
	int V,E; scanf("%d %d",&V,&E);
	for (int i=0;i<E;i++){
		int left,right; scanf("%d %d",&left,&right);
		v[left].push_back(right);
	}

	for (int i=0;i<V;i++)
		if(!finished[i+1])
			scc(i+1);

	sort(res.begin(),res.end());

	printf("%lu\n",res.size());
	for (int i=0;i<res.size();i++){
		vector<int> temp = res[i];
		for (int j=0;j<temp.size();j++){
			printf("%d ",temp[j]);
		}
		printf("-1\n");
	}

	return 0;
}

