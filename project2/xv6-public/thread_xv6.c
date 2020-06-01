#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_THREAD (9)
#define NUM_INCREASE (10000)

int global_cnt = 0;

struct thread_mutex_t{
	struct spinlock lock;
};

typedef struct thread_mutex_t thread_mutex_t;

thread_mutext_t my_lock;

int ret_val[NUM_THREAD];

void* increase(void* arg){
	int* int_arg = (int*) arg;
	//int ret = 200;
	printf("[%d Thread] pid[%d] %d\n",thread_self(), getpid(),int_arg[1]);

	//printf("%d\n",int_arg[1]);
	for (int i=0;i<int_arg[0];i++){
		thread_mutext_lock(&my_lock);
		global_cnt++;
		thread_mutext_unlock(&my_lock);
	}

	thread_exit(&int_arg[1]);
	//return (void*) int_arg[1];
}

void my_error(const char* str){
	printf("[ERROR] %s\n",str);
}

int main(int argc, char const *argv[])
{

	thread_t pid[10];
	thread_mutex_init(&my_lock,NULL);

	printf("[%s Thread] pid[%d]\n","Master",getpid());

	int arg[NUM_THREAD][2];
	for (int i=0;i<NUM_THREAD;i++)
		arg[i][0] = NUM_INCREASE;

	for (int i = 0; i < NUM_THREAD; i++){
		arg[i][1] = (i+1)*(i+1);
		int ret = thread_create(&pid[i], NULL, &increase, (void*)arg[i]);
		if(!ret)
			continue;
		else
			my_error("thread_create");
	}
    int *ptr[NUM_THREAD];

	void *ret_val[10];
	for (int i=0;i<NUM_THREAD;i++){
	    thread_join(pid[i], (void**)&(ptr[i]));

	    printf("\n return value from first thread is [%d]\n", *ptr[i]);
	}

	printf("%d\n",global_cnt);

	return 0;
}
