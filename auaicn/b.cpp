#include <bits/stdc++.h>
using namespace std;
bool visitede[100001],visitedc[1001];
vector<vector<int>> cluster,station;

int main(){
    int n,k,m;
    scanf("%d %d %d",&n,&k,&m);
    vector<vector<int>> cluster(n+1),station(m+1);
    for(int i=1;i<=m;i++){
        for(int j=0;j<k;j++){
            int tmp;
            scanf("%d",&tmp);
            station[i].push_back(tmp);
            cluster[tmp].push_back(i);
        }
    }
    if(n==1){
        printf("1\n");
        return 0;
    }
    queue<int> q;
    visitede[1] = true;
    for(int c: cluster[1]){
        if(!visitedc[c]){
            visitedc[c] = true;
            for(int e: station[c]){
              if(!visitede[e]){
                    visitede[e] = true;
                    q.push(e);
                }
            }
        }
    }
    int ret=1;
    while(!q.empty()){
        int s = q.size();
        while(s--){
            int cur = q.front();
            q.pop();
            if(cur==n){
                printf("%d\n",ret+1);
                return 0;
            }
            for(int c: cluster[cur]){
                if(!visitedc[c]){
                    visitedc[c] = true;
                    for(int e: station[c]){
                        if(!visitede[e]){
                            visitede[e] = true;
                            q.push(e);
                        }
                    } 
                }
            }
        }
        ret++;
    }
    printf("-1\n");
    return 0;
}