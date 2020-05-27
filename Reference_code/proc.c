#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

/* Project 1 */


enum { MAX_MLFQ_LEV = 3 };

const uint TIME_QUANTUM[MAX_MLFQ_LEV] = {1, 2, 4};
const uint TIME_ALLOTMENT[MAX_MLFQ_LEV] = {5, 10, UINT_MAX};

uint boost_tick = 0;							// Logical tick counter for priority boost	

uint total_stride_ticket = 0;			// Total number of tickets stride processes have

int havestride = 0;								// If non-zero, number of stride processes

struct {
	/* simple circular queue for MLFQ */
	struct proc* queue[MAX_MLFQ_LEV][NPROC];
	uint queue_front[MAX_MLFQ_LEV];
	uint queue_rear[MAX_MLFQ_LEV];

	uint mlfq_pass;			// MLFQ's pass value
	uint mlfq_stride;		// MLFQ's stride
	uint mlfq_ticket;		// MLFQ's tickets

} mlfq_proc;

void
minit(void)
{
	mlfq_proc.mlfq_pass = 0;
	mlfq_proc.mlfq_ticket = TOTAL_TICKET;	 
	mlfq_proc.mlfq_stride = LARGENUM / mlfq_proc.mlfq_ticket; 

	for (int i = 0; i < 3; ++i) {
		mlfq_proc.queue_front[i] = 0;
		mlfq_proc.queue_rear[i] = 0;

		for (int np = 0; np < NPROC; np++) {
			mlfq_proc.queue[i][np] = 0;
		}
	}
}

void
mlfq_pass_inc(void)
{
	mlfq_proc.mlfq_pass += mlfq_proc.mlfq_stride;
}

static int
is_full(int lev)
{
	return mlfq_proc.queue_front[lev] == ((mlfq_proc.queue_rear[lev] + 1) % NPROC);
}

static int
is_empty(int lev)
{
	return mlfq_proc.queue_front[lev] == mlfq_proc.queue_rear[lev];
}

static void
enqueue(int lev, struct proc* proc)
{
	if (is_full(lev))
		panic("Queue is already full");
	mlfq_proc.queue_rear[lev] = ((mlfq_proc.queue_rear[lev] + 1) % NPROC);
	mlfq_proc.queue[lev][mlfq_proc.queue_rear[lev]] = proc;
}

static struct proc*
dequeue(int lev)
{
	if(is_empty(lev)) {
		return (struct proc*)0;
	}
	mlfq_proc.queue_front[lev] = ((mlfq_proc.queue_front[lev] + 1) % NPROC);
	return mlfq_proc.queue[lev][mlfq_proc.queue_front[lev]];
}

int
getlev(void)
{
	return myproc()->lev;
}

static void
halt(void)
{
	asm volatile("hlt");
}

void
priority_boost(void)
{
  struct proc* p;

  if (boost_tick < BOOST_LIMIT)
    panic("panic in priority boost");

  boost_tick = 0; 
 	int flag = holding(&ptable.lock); 
	if (!flag)
  	acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->is_stride) {
      continue;
    }

    if (p->lev == 0)
			continue;

		p->lev = 0; 
    p->tick_cnt = 0; 
  }
  // Dequeue all levels except highest, and enqueue them Lev 0
  for(;;) {
    if (!(p = dequeue(1))) { 
      break;
    }
    enqueue(0, p);    
  }

  for (;;) {
    if (!(p = dequeue(2))) { 
      break;
    }
    enqueue(0, p);    
  }

	if (mlfq_proc.queue_front[1] != mlfq_proc.queue_rear[1] ||
				mlfq_proc.queue_front[2] != mlfq_proc.queue_rear[2])
		panic("panic in priority boost().");
	if (!flag)
		release(&ptable.lock);
}


static struct proc*
find_mlfq_proc(void)
{
	struct proc* p;
	int lev = 0;
	for (;;) {
		if (!(p = dequeue(lev))) {
			lev++;
			break;
		}
		return p;
	}

	for (;;) {
		if (!(p = dequeue(lev))) {
			lev++;
			break;
		}
		return p;
	}

	for (;;) {
		if (!(p = dequeue(lev))) {
			break;
		}
		return p;
	}
	return (struct proc*)0;
}
// Give up the CPU for one scheduling round.
int
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
	struct proc* p = myproc();
	
	// Increase boost_tick 
	boost_tick += 1;
	if (boost_tick >= BOOST_LIMIT) {
		priority_boost();
	}

	// Increase process's tick_cnt
	p->tick_cnt += 1;

	// Increase its/MLFQ's pass value
	if (p->is_stride) {
		p->pass_value += p->stride;
	} else {
		// increase priority level as needed
		if (p->lev < MAX_MLFQ_LEV - 1 && p->tick_cnt >= TIME_ALLOTMENT[p->lev]) {
			p->lev++;
			p->tick_cnt = 0;
		} else if (p->lev == MAX_MLFQ_LEV - 1 && p->tick_cnt >= TIME_QUANTUM[p->lev]) {
			p->tick_cnt = 0;
		}

		enqueue(p->lev, p);
		mlfq_pass_inc();
	}
	 
  sched();
  release(&ptable.lock);
	return 0;
}

static uint
get_min_pass()
{
	// The ptable lock must be held
	if(!holding(&ptable.lock))
		panic("panic in get_min_pass().");
	
	uint min_pass = mlfq_proc.mlfq_pass;
	struct proc* p;
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->is_stride && p->state == RUNNABLE && p->pass_value < min_pass) {
			min_pass = p->pass_value;
		}
	}
	return min_pass;
}

static void
init_pass()
{
	mlfq_proc.mlfq_pass = 0 ;
	struct proc* p;
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->stride)
			p->pass_value = 0;
	}
}

int
set_cpu_share(int ticket)
{
	if (ticket + total_stride_ticket > MAX_STRIDE_TICKET)
		return -1;

	acquire(&ptable.lock);

	if (ticket + total_stride_ticket > MAX_STRIDE_TICKET) {
		release(&ptable.lock);
		return -1;
	}

	struct proc* p = myproc();
	
	havestride++;	
	total_stride_ticket += ticket;

	p->lev = -1;
	p->tick_cnt = 0;
	p->is_stride = 1;
	p->ticket = ticket;
	p->stride = LARGENUM / ticket;
	p->pass_value = get_min_pass();

	mlfq_proc.mlfq_ticket -= ticket;
	mlfq_proc.mlfq_stride = LARGENUM / mlfq_proc.mlfq_ticket;

	if (mlfq_proc.mlfq_ticket + total_stride_ticket != TOTAL_TICKET)
		panic("panic in set_cpu_share().");
	
	release(&ptable.lock);
	return 1;
}

static struct proc*
find_stride_proc(uint min_pass)
{
	struct proc* p;
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->state != RUNNABLE || !p->is_stride || p->pass_value != min_pass)
			continue;
		return p;
	}
	panic("panic in find_stride_proc().");
}

/* Project 1 */

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
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
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

	p->lev = 0;
	p->tick_cnt = 0;
	p->is_stride = 0;
	p->stride = 0;
	p->pass_value = 0;
	p->ticket = 0;

	
  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;
	enqueue(p->lev, p);

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;
	
	np->lev = 0;
	np->tick_cnt = 0;
	np->is_stride = 0;
	np->stride = 0;
	np->pass_value = 0;
	np->ticket = 0;

  acquire(&ptable.lock);

  np->state = RUNNABLE;
	enqueue(np->lev, np);
	
  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

	
	if (curproc->is_stride) {
		havestride--;
		total_stride_ticket -= curproc->ticket;

		mlfq_proc.mlfq_ticket += curproc->ticket;
		mlfq_proc.mlfq_stride = LARGENUM / mlfq_proc.mlfq_ticket;
	}
	
  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;

				p->lev = 0;
				p->tick_cnt = 0;
				p->is_stride = 0;
				p->stride = 0;
				p->pass_value = 0;
				p->ticket = 0;

        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
	uint min_pass;
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

		min_pass = get_min_pass();

		if (min_pass == mlfq_proc.mlfq_pass) {
			p = find_mlfq_proc();
			if (!p) {
				mlfq_pass_inc();
				release(&ptable.lock);
				halt();
				continue;
			}
		} else {
			p = find_stride_proc(min_pass);
			if (!p)
				panic("panic in scheduler(). Cannot find stride process.");
		}

		if (mlfq_proc.mlfq_pass >= MAX_PASS && min_pass >= MAX_PASS) {
			init_pass();
		}

		if (p) {
			c->proc = p;
			switchuvm(p);
			p->state = RUNNING;
			swtch(&(c->scheduler), p->context);
			switchkvm();
			
			c->proc = 0;
		}
		
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
// This is not system call, see yield()
void
tick_yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  struct proc* p = myproc();
	myproc()->state = RUNNABLE;
	
	if (!p->is_stride) {
		if (p->lev < MAX_MLFQ_LEV - 1 && p->tick_cnt >= TIME_ALLOTMENT[p->lev]) {
			p->lev++;
			p->tick_cnt = 0;
		} else if (p->lev == MAX_MLFQ_LEV - 1) {
			p->tick_cnt = 0;
		}
		enqueue(p->lev, p);
	}

  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      if (!p->is_stride) {
				enqueue(p->lev, p);
			} else {
				p->pass_value = get_min_pass();
			}

			p->state = RUNNABLE;
		}
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING) {
				if (!p->is_stride) {
					enqueue(p->lev, p);
				} else {
					p->pass_value = get_min_pass();
				}
        p->state = RUNNABLE;
			}
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
