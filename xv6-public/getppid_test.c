#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc,char* argv[]){
	int pid_ = getpid();
	int ppid_ = getppid();
	printf(2,"pid is %d\nppid is %d\n",pid_,ppid_);
	exit();
}
