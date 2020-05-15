#include "types.h"
#include "defs.h"
#include "thread.h"

// thread.c
int 
thread_create(thread_t *thread, void* (*start_rountine)(void*), void *arg)
{

	return 0;
}

void 
thread_exit(void *retval)
{

}

int
thread_join(thread_t thread, void **retval)
{
	return 0;
}
