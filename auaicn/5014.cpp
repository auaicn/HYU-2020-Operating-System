#include <stdio.h>
#include <iostream>
#include <queue>

using namespace std;

bool visit[1000010];

int main(int argc, char const *argv[])
{
	int F,S,G,U,D; scanf("%d %d %d %d %d",&F,&S,&G,&U,&D);
	queue<pair<int,int> > q;
	q.push({S,0});
	while(!q.empty()){
		pair<int,int> pos = q.front(); q.pop();
		int now = pos.first;
		int res = pos.second;
		if(now==G){
			printf("%d\n",res);
			return 0;
		}
		int left = now-D;
		int right = now+U;
		if(right<=F)
			if(!visit[right]){
				q.push({right,res+1});
				visit[right] = true;
			}
		if(left>=1)
			if(!visit[left]){
				q.push({left,res+1});
				visit[left] = true;
			}
	}
	printf("use the stairs\n");
	return 0;
}