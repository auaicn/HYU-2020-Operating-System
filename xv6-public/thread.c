#include "types.h"
#include "defs.h"
#include "thread.h"

// thread.c
int 
thread_create(thread_t *thread, void* (*start_rountine)(void*), void *arg)
{
	struct thread* new_thread;
	struct proc* p = myproc();
	char* sp;

	acquire(&ptable.lock);
	for (new_thread = p->threads; new_thread < &(p->threads[NTHREAD]);new_thread++)
		if(new_thread->state == UNUSED)
			goto found;

	// not found
	release(&ptable.lock);
	return 0;

found:
	new_thread -> state = EMBRYO;
	new_thread -> tid = (p -> ++num_thread);

	release(&ptable.lock);

	if((new_thread -> kstack = kalloc()) == 0){
		new_thread -> state = UNUSED;
		return 0;
	}

	sp = new_thread -> kstack + KSTACKSIZE;


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
