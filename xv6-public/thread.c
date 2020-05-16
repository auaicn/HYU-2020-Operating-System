#include "types.h"
#include "defs.h"
#include "proc.h"

int thread_self(void){
	return myproc()->tid;
	// master thread gets -1
	// working threads gets unique_by_process number.
	// its' returned value can be same in different process.
};

int thread_mutex_lock(struct thread_mutex_t* mutex_structure){
	acquire(&mutex_structure.lock); // double acquire -> panic
	return 0;
};

int thread_mutex_unlock(struct thread_mutex_t* mutex_structure){
	release(&mutex_structure.lock); // can panic
	return 0;
};

int thread_mutex_init(struct thread_mutex_t* mutex_structure, char* name){
	initlock(&(myproc()->thread[0]->lock),"thread");
	return 0;
};

struct proc*
allocthread(void* (*start_rountine)(void*))
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = -1;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){

    p->state = UNUSED;
    
    // fail return
    return 0;
  }

  // Q. why not using exact p->kstack value?
  // A. free list maintains, low value.
  //    so go the high value again.
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame. type casting.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)start_rountine;

  acquire(&ptable.lock);
  p->threads[(++p->num_thread[0])] = p;
  release(&ptable.lock);

  cprintf("PROC ALLOCED with pid[%d]\n",p->pid);

  return p;
}

// thread.c
int 
thread_create(thread_t *thread, void* (*start_rountine)(void*), void *arg)
{
	struct proc* p;
	struct thread* new_thread;

	// TO ejfpaosf
	proc* p = myproc();

  	if((new_thread = allocthread()) == 0)
    	return -1;

    p -> classifier = 1;
	new_thread -> classifier = 2;

	new_thread -> pid = -1;
	new_thread -> tid = ++(p -> num_thread);

	new_thread -> context -> eip = (void*)start_rountine;

	new_thread -> sp -= sizeof(arg);
	cpy(arg,sp,sizeof(sp));
	new_thread -> sp += sizeof(arg);

	//
	pde_t* pgdir;                // Page table
  
	char *kstack;                // Bottom of kernel stack for this process
	enum procstate state;        // Process state
	// allocprock then returned with EMBRYO state
	

	struct trapframe *tf;        // Trap frame for current syscall
	struct context *context;     // swtch() here to run process
	void *chan;                  // If non-zero, sleeping on chan
	
	// int killed;                  // If non-zero, have been killed
	
	// struct inode *cwd;           // Current directory
	curproc -> cwd = p ->cwd;

	// FORK
	// uint sz;                     // Size of process memory (bytes)
	new_thread->sz = curproc->sz;
	
	// struct proc *parent;         // Parent process
	new_thread->parent = curproc;
	
	// Seriously?
	// struct trapframe *tf;        // Trap frame for current syscall
	*new_thread->tf = *curproc->tf;

	// Clear %eax so that fork returns 0 in the child.
	new_thread->tf->eax = 0;

	// struct file *ofile[NOFILE];  // Open files
	for(i = 0; i < NOFILE; i++)
		if(curproc->ofile[i])
			new_thread->ofile[i] = filedup(curproc->ofile[i]);
	new_thread->cwd = idup(curproc->cwd);

  	// char name[16];               // Process name (debugging)
	safestrcpy(new_thread->name, curproc->name, sizeof(curproc->name));

	// to return in fork but doesnt need here. pid = new_thread->pid;

	acquire(&ptable.lock);

	new_thread->state = RUNNABLE;

	release(&ptable.lock);
	// FORK

	return 0;
}

void 
thread_exit(void *retval)
{
	struct proc * curproc;

	curproc = myproc();
	
	// k stack now unavailable
	free(curproc -> kstack);

	// working thread become zombie. exit of master thread will deal with it
	curproc -> state = ZOMBIE;
	
	// tid reused as retval
	curproc -> tid = *retval;

	wakeup1(curproc->parent);

	// after signal, parent can know the retval through tid.
	sched();
	panic("[THREAD]zombie exit");
}

int
thread_join(thread_t thread, void **retval)
{
	return 0;
}
