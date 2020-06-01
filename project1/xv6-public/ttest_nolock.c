#include "types.h"
#include "stat.h"
#include "user.h"

#ifndef NULL
#define NULL		 ((void*)0)
#endif

#define NUM_THREAD (1)
#define NUM_INCREASE (10000000)

int global_cnt = 0;

int ret_val[NUM_THREAD];

void* increase(void* arg){
	int* int_arg = (int*) arg;	
	printf(1,"gcnt[%d]\n",global_cnt);
	printf(1,"arg[%d]\n",*int_arg);
	// printf(1,"[%d Thread] pid[%d] %d\n",thread_self(), getpid(),int_arg[1]);
	// printf(1,"[%d Thread] pid[%d] %d\n",97, getpid(),int_arg[1]);
	
	for (int i=0;i<int_arg[0];i++){
		printf(1,"+");
		global_cnt++;
	}

	thread_exit(&ret_val[1]);
	return NULL;
}

void my_error(const char* str){
	printf(1,"[ERROR] %s\n",str);
}

int arg[NUM_THREAD];


int main(int argc, char const *argv[])
{

	thread_t pid[10];
	//thread_mutex_init(&my_lock,NULL);

	printf(1,"[%s Thread] pid[%d]\n","Master",getpid());
	//printf(1,"start routine in user, integer %d\n",(int)increase);

	for (int i=0;i<NUM_THREAD;i++)
		arg[i] = NUM_INCREASE;

	for (int i = 0; i < NUM_THREAD; i++){
		int ret = thread_create(&pid[i], &increase, &arg[i]);
		if(!ret)
			continue;
		else
			my_error("thread_create");
		printf(1,"MAIN thread created with tid[%d]",arg[i]);
	}
    int *ptr[NUM_THREAD];

	//void *ret_val[10];
	for (int i=0;i<NUM_THREAD;i++){
	    thread_join(pid[i], (void**)&(ptr[i]));

	    printf(1,"\n return value from first thread is [%d]\n", *ptr[i]);
	}

	printf(1,"%d\n",global_cnt);

	return 0;
}
