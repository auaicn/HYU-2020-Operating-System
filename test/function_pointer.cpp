#include <stdio.h>
#include <iostream>

using namespace std;

int (*temp)(int,int){
	printf("hello\n");
};

void hello(){
	printf("hello\n");
}

int max(int x, int y){
	return x>y?x:y;
}


int main(int argc, char const *argv[])
{
	//temp = &max;
	//printf("%d",temp(10,1));
	//printf("%d\n",max(10,1));
	return 0;
}