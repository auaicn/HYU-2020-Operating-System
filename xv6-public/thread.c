#include "types.h"
#include "defs.h"

// thread.c
int 
thread_create(thread_t *thread, void* (*start_rountine)(void*), void *arg)
{
	struct proc* p;
	struct thread* new_thread;

	// TO ejfpaosf
	proc* p = myproc();

  	if((new_thread = allocproc()) == 0)
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

	// STARTS
	curproc = myproc();
	free(curproc -> kstack);
	curproc -> state = ZOMBIE;

	/*
	// Close all open files.
	for(fd = 0; fd < NOFILE; fd++){
	if(curproc->ofile[fd]){
	  fileclose(curproc->ofile[fd]);
	  curproc->ofile[fd] = 0;
	}

	begin_op();
	iput(curproc->cwd);
	end_op();
	curproc->cwd = 0;

	acquire(&ptable.lock);

	*/
	// Parent might be sleeping in wait().
	wakeup1(curproc->parent);

	/*
	// Pass abandoned children to init.
	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
	if(p->parent == curproc){
	  p->parent = initproc;
	  if(p->state == ZOMBIE)
	    wakeup1(initproc);
	}
	}
	*/


	curproc->state = ZOMBIE;
	sched();
	panic("[THREAD]zombie exit");
}

int
thread_join(thread_t thread, void **retval)
{
	return 0;
}
