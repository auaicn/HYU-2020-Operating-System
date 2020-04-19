#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc,char* argv[]){
	int child = fork();
	if(child==0){
		for(;;){
			printf(0,"CHILD\n");
			yield();
		}

	}else if (child>0){
		for(;;){
			printf(0,"PARENT\n");
			yield();
		}

	}else{
		printf(0,"fork error\n");
	}
	exit();
}
