#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREAD (9)
#define NUM_INCREASE (10000)

int global_cnt = 0;

pthread_mutex_t my_lock;

int ret_val[NUM_THREAD];

void* increase(void* arg){
	int* int_arg = (int*) arg;
	//int ret = 200;
	printf("[%d Thread] pid[%d] %d\n",pthread_self(), getpid(),int_arg[1]);

	//printf("%d\n",int_arg[1]);
	for (int i=0;i<int_arg[0];i++){
		pthread_mutex_lock(&my_lock);
		global_cnt++;
		pthread_mutex_unlock(&my_lock);
	}

	pthread_exit(&int_arg[1]);
	//return (void*) int_arg[1];
}

void my_error(const char* str){
	printf("[ERROR] %s\n",str);
}

int main(int argc, char const *argv[])
{

	pthread_t pid[10];
	pthread_attr_t pattr;
	pthread_mutex_init(&my_lock,NULL);

	printf("[%s Thread] pid[%d]\n","Master",getpid());

	int arg[NUM_THREAD][2];
	for (int i=0;i<NUM_THREAD;i++)
		arg[i][0] = NUM_INCREASE;

	for (int i = 0; i < NUM_THREAD; i++){
		arg[i][1] = (i+1)*(i+1);
		int ret = pthread_create(&pid[i], NULL, &increase, (void*)arg[i]);
		if(!ret)
			continue;
		else
			my_error("pthread_create");
	}
    int *ptr[NUM_THREAD];

	void *ret_val[10];
	//void *ret_val;
	for (int i=0;i<NUM_THREAD;i++){
	    pthread_join(pid[i], (void**)&(ptr[i]));

		//pthread_join(pid[i],ret_val[i]);
		//pthread_join(pid[i],&ret_val);

	    printf("\n return value from first thread is [%d]\n", *ptr[i]);

		//printf("[%2d Thread] RETURNS %d\n",pid[i],((int*)ret_val)[i]);
		//printf("[%2d Thread] RETURNS %d\n",pid[i], *((int*)ret_val));
	}

	/*
	for (int i=0;i<NUM_THREAD;i++)
	*/
	printf("%d\n",global_cnt);
	/* code */
	return 0;
}
