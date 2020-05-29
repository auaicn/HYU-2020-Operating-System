#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

#define MULTSTRIDESHARE (1000)
#define DEPTH_QUEUE   (3)
#define PERIOD_BOOSTING (200)
#define STRIDE_DEFAULT_TIME_QUANTOM (5)
#define INITIAL_LEV (0)
#define STRIDE_LEV (9)


struct ptable_struct{
struct spinlock lock;
struct proc proc[NPROC];
struct proc* ARRAYQUEUE[DEPTH_QUEUE][NPROC];
int q_size[DEPTH_QUEUE];
} ptable;

struct {
struct proc* proc[NPROC]; 
int stable_size;
} stable;

static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);
extern void trapret(void);
static void wakeup1(void *chan);
void do_nothing(void);


int min_pass;

/*      THREAD        */

/*
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

*/

thread*
allocthread(void)
{
    struct proc *p;
    struct proc* curproc = myproc(); // caller, master thread

    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if(p->state == UNUSED)
            goto found;

    release(&ptable.lock);
    cprintf("UNUSED ptable entry not detected\n");
    return 0;

found:
    p->state = EMBRYO;
    p->pid = -1; // thread representation.

    release(&ptable.lock);

    // Allocate kernel stack.
    if((p -> kstack = kalloc()) == 0){
        p -> state = UNUSED;
        return 0;
    }

    // memset(p -> kstack, 0, PGSIZE);
    p -> pgdir = curproc -> pgdir;  

    //mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
    /*
    if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
        cprintf("allocuvm out of memory (2)\n");
        deallocuvm(pgdir, newsz, oldsz);
        kfree(mem);
        return 0;
    }
    if(mappage(p->pgdir, p->kstack + PGSIZE, 4096, V2P(p->kstack), PTE_W)<0){
        cprintf("allocuvm out of memory (2)\n");
        kfree(p->kstack);
        return 0; // failure
    }
    */

    char *sp;
    sp = p -> kstack + KSTACKSIZE;

    // TRAPFRAME setting
    sp -= sizeof *p->tf;
    p -> tf = (struct trapframe*)sp;
    *p->tf = *curproc->tf;
   // memset(p->tf, 0, sizeof *p->tf);

    // where to go after context change, forkret returns.
    sp -= 4;
    *(uint*)sp = (uint)trapret;

    // Set up new context to start executing at forkret,
    sp -= sizeof *p->context;
    p -> context = (struct context*)sp;
    *(p->context) = *(curproc->context);

    // trapfram and context
    // 1. typecast
    // 2. copy master at first and then lets overwrite

    // cprintf("fret %x\n",forkret);
    // p -> context -> eip = (uint)forkret;
    p -> context -> eip = (uint)forkret;
    p -> first_scheduled = 1;
    // p -> context -> esp = sp;

    thread* t = (thread*) p;
    return t;

}

int 
thread_create(thread_t *thread, void* (*start_routine)(void*), void *arg)
{

    struct proc* nt;
    struct proc* p = myproc();

    cprintf("pid[%d] called thread_create\n",p->pid);

    if((nt = allocthread()) == NULL)
        return -1;  
    
    nt -> multi_threaded = 1;
    nt -> master_thread = p;

    // oroginal proc structure variables
    nt -> chan = 0;
    nt -> killed = 0;
    nt -> cwd = p -> cwd;
    nt -> num_thread = -1;

    // threads share pg dir
    // actually essence of 

    // copy of size
    // used in allocuvm() call to get changed size and check them.
    int sz = p -> sz;

    // actually, rounded up value is alreay stored. (round not needed)
    sz = PGROUNDUP(sz);
    nt -> pgdir = p -> pgdir;

    if((sz = allocuvm(nt -> pgdir, sz, sz + 2 * PGSIZE)) == 0)
        panic("allocuvm");

    // parent has to know increased user memory size.
    // thread cannot be more than one for now
    // apply new total allocated size
    p -> sz = sz;
    nt -> sz = sz;

    // cprintf("changed master,new thread sz:%d\n",p->sz);

    // top of kstack is saved in kstack var.
    // other process's (init,bash) is observed to have
    // lowest 3 bit equals 0 (in hex representation)
    // 'bottom' is more exact explanation maybe.
    // 'bottom' has highest value. esp decreases as stack grows.
    // nt -> kstack = (char*)sz;

    // newsz = oldsz + 2*PGSIZE
    // [oldsz:oldsz+PGSIZE] cleared
    // [oldsz+PGSIZE:oldsz+2PGSIZE] will be used as own user stack for new thread.
    // reusable maybe but later.
    // clearpteu(p->pgdir,(char*)(p -> sz - 2*PGSIZE));

    // cprintf("curproc eip %x\n",p->context->eip);


    int argc = 1;

    // only one argument available for now.
    int ustack[1 + 1];

    // int* argv = (int*) arg;
    // ustack[0] = (uint)thread_exit; // auto clear it not called explicitly
    ustack[0] = 0xFFFFFFFF;// (int)thread_exit;
    ustack[1] = (uint) ((int*)arg)[0]; 

    // ustack[2] = (uint) ((int*)arg)[1]; 
    // cprintf("u %d %d\n",ustack[0],ustack[1]);
    // well done

    if(copyout(nt -> pgdir, nt -> tf -> esp, ustack, (argc+1) * 4) < 0)
        panic("copyout");

    // tf -> esp has to point user stack!!? maybe yes.
    // 'top' of stack.
    // two elements in. 
    // one for fake (or later thread_exit())
    // another for argument passed from user of master_thread.
    nt -> tf -> esp = p -> sz;
    nt -> tf -> esp -= (argc + 1) * sizeof(int);

    // routine to execute after it goes to user mode
    nt -> tf -> eip = (uint)start_routine;


    // file duplication. same with fork
    for(int i = 0; i < NOFILE; i++)
        if(p->ofile[i])
            nt->ofile[i] = filedup(p->ofile[i]);
    nt->cwd = idup(p->cwd);

    // for debugging
    safestrcpy(nt->name, p->name, sizeof(p->name));

    // critical section
    // proc.multi_threaded variable is ciritcal to flow 
    // in 'trap.c' and 'scheduler'
    acquire(&ptable.lock);
    p -> multi_threaded = 1; // critical in flow

    // number of thread and table of threads adjusted here
    // or table adjusting would intervened by lookup in the threads.
    p -> threads[0] = p;
    p -> num_thread++;
    p -> threads[p -> num_thread] = nt;
    nt -> tid = p -> num_thread;
    nt -> state = RUNNABLE;
    release(&ptable.lock);

    // master
    cprintf("[master info]  tid[%d] ",p->tid);
    cprintf("master pid[%d] sz[%x] kstack[%x] ",p->master_thread->pid,p->sz,p->kstack);
    cprintf("pgdir[%x]\n",p->pgdir);

    // working
    cprintf("[thread allo]  tid[%d] pid[%d] ",nt->tid,nt->pid);
    cprintf("master pid[%d] sz[%x] kstack[%x] ",nt->master_thread->pid,nt->sz,nt->kstack);
    cprintf("pgdir[%x]\n",nt->pgdir);

    *thread = nt->tid;
    //cprintf("retval : %d\n,",*thread);

    return 0; // success
}

void 
thread_exit(void *retval)
{
    struct proc * curproc;
    curproc = myproc();

    if(curproc->tid == 0)
        panic("master thread calling thread_exit\n");

    cprintf("tid[%d] called thread_exit\n\n\n", myproc()->tid);
    
    /* exit() */
    // Close all open files.
    int fd;
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
    /* exit() */

    cprintf("acquring lock\n");

    acquire(&ptable.lock);

    wakeup1(curproc->master_thread);
    
    struct proc* p;
    for (int i=0;i<=curproc->master_thread->num_thread;i++){
        p = curproc->master_thread->threads[i];
        
        p -> parent = initproc;
        if (p-> state == ZOMBIE)
            wakeup1(initproc);
    }

    // working thread become zombie. exit of master thread will deal with it
    curproc -> state = ZOMBIE;
    curproc -> master_thread -> ret_val[curproc->tid] = *(int*)retval;
    
    wakeup1(curproc->parent);

    // call sched() with lock
    sched();
    panic("[THREAD]zombie exit");

    // NO return
    // more precisely, must not scheduled and returned.
    // its' state is ZOMBIE, not to be scheduled.
}

int
thread_join(thread_t threadid, void **retval)
{

    struct proc * curproc;
    curproc = myproc();
    cprintf("entered join\n");
    // cleanup Kernel stack and User Stack
    // kfree(curproc -> kstack);

    // kfree(curproc -> sz);
    // Never. Violation could happen later.
    // I'm using Master thread(initially Single Process)'s Userstack
    // as thread's own stack.
    if(curproc -> tid != 0)
        panic("working thread calling join\n");

    cprintf("master goinging to sleep\n");
    int found = 0;
    int gosleep = 1;

    if(curproc->threads[threadid]->state == ZOMBIE){
        gosleep = 0;
        found = 1;
        cprintf("working thread already terminated. immediate return\n");
    }

    // thread table adjustment here.
    for(;;){

        // acquiresleep(&ptable.lock);
        if(gosleep){
            cprintf("master woke from sleep\n");
            sleep(curproc,NULL);
            cprintf("master woke from sleep\n");
            acquire(&ptable.lock);
        }
        // other thread's signal could wake up this process.
            
        thread* lookup = curproc->threads[threadid];
        cprintf("lookup : state %d, tid %d\n",lookup->state,lookup->tid);
        if(lookup->state == ZOMBIE){
            found = 1;
            kfree(lookup->kstack);
            *(int*)retval = curproc -> ret_val[threadid];

            // adjusting thread table
            curproc->num_thread--;
            for (int j=threadid;j<curproc->num_thread;j++){
                curproc->threads[j] = curproc->threads[j+1];
                curproc->threads[j] -> tid = j;
                curproc->ret_val[j] = curproc->ret_val[j+1];
            }

            if(curproc->num_thread == 0)
                curproc->multi_threaded = 0;

            // deallocuvmcurproc->pgdir,old,now()

            release(&ptable.lock);
        }

        if(found)
            break;

    }
    cprintf("thread exit escaping\n");

    return 0; 
    // success
}




/*      THREAD        */


void
queue_table_lookup(void)
{
    for (int i=0;i<DEPTH_QUEUE;i++){
    cprintf("[LEV%d]SIZE[%d] : ",i ,ptable.q_size[i]+1);
    for (int j=0;j<=ptable.q_size[i];j++)
    cprintf("%d[%d] ",ptable.ARRAYQUEUE[i][j]->pid,ptable.ARRAYQUEUE[i][j]->state);
    cprintf("\n");
    }
    }

int 
min(int x,int y)
{
    return x<y?x:y;
}

int
getlev(void)
{
    return myproc()->lev;
}

int
set_cpu_share(int share)
{

    struct proc* mlfq = stable.proc[0];

    if (mlfq -> share - share < 20 || share < 0){
        cprintf("proc.c:set_cpu_share:: unable to set cpu share\n");
        return -1;
    }

    acquire(&ptable.lock);
    struct proc* p= myproc();

    if(p -> tid)
        p = p -> parent;

    stable.proc[++stable.stable_size] = p;

    mlfq -> share -= share;
    p -> share = share;
    p -> pass = min_pass;

    // dequeue from q
    for (int i=0;i<=ptable.q_size[p->lev];i++){
        if(p == ptable.ARRAYQUEUE[p->lev][i]){
            ptable.q_size[p->lev]--;
            for (int j=i;j<=ptable.q_size[p->lev];j++)
                ptable.ARRAYQUEUE[p->lev][j] = ptable.ARRAYQUEUE[p->lev][j+1];
            break;
        }
    }

    // clear variables associated to MLFQ
    p->lev = STRIDE_LEV;
    p->time_allotment = STRIDE_DEFAULT_TIME_QUANTOM;

    release(&ptable.lock);
    return 1;
}

    // until here

void
pinit(void)
{
    initlock(&ptable.lock, "ptable");
}

    // Must be called with interrupts disabled
int
cpuid() 
{
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

    // LWP
    p -> multi_threaded = 0; // false initially
    p -> num_thread = 0;

    // for MLFQ scheduling
    p -> lev = INITIAL_LEV;
    p -> time_allotment = time_allotment[INITIAL_LEV];
    p -> start_tick = ticks;

    // for STRIDE scheduling
    p -> share = 0;
    p -> pass = 0;

    // debugging testcase
    p -> first_scheduled = 0;
    p -> master_thread = p;
    p -> tid = 0;

    p->last_visited = 0;

    acquire(&ptable.lock);
    ptable.ARRAYQUEUE[0][++ptable.q_size[0]] = p;
    release(&ptable.lock);

    return p;
}

    //PAGEBREAK: 32
    // Set up first user process.
void
userinit(void)
    {
    //cprintf("[USERINIT]Entering\n");
    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    // again implement just queue, not by linked list
    for (int i = 0; i < DEPTH_QUEUE; i++)
    ptable.q_size[i] = -1;
    stable.stable_size = -1;

    // pid 1 process
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

    // this assignment to p->state lets other cores
    // run this process. the acquire forces the above
    // writes to be visible, and the lock is also needed
    // because the assignment might not be atomic.
    acquire(&ptable.lock);

    p->state = RUNNABLE;

    release(&ptable.lock);

    cprintf("[init process allocated] with pid[%d] ",p->pid);
    cprintf("master pid[%d] sz[%x] kstack[%x] ",p->master_thread->pid,p->sz,p->kstack);
    cprintf("pgdir[%x]\n",p->pgdir);

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
    // cprintf("[fork]oldpgdir %x, newpgdir %x\n",curproc->pgdir,np->pgdir);

    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;

    for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
    np->ofile[i] = filedup(curproc->ofile[i]);
    np->cwd = idup(curproc->cwd);

    safestrcpy(np->name, curproc->name, sizeof(curproc->name));

    // to return
    pid = np->pid;

    // MASTER thread
    np -> num_thread = 0;
    np -> tid = 0;

    acquire(&ptable.lock);
    np->state = RUNNABLE;
    release(&ptable.lock);

    
    cprintf("[fork process allocated] with pid[%d] ",np->pid);
    cprintf("master pid[%d] sz[%x] kstack[%x] ",np->master_thread->pid,np->sz,np->kstack);
    cprintf("pgdir[%x]\n",np->pgdir);


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

    //cprintf("exiting\n");
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

    // current process may
    // Pass abandoned children to init.
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->parent == curproc){
            p->parent = initproc;
        if(p->state == ZOMBIE)
            wakeup1(initproc);
        }
    }

    struct proc* t;
    if(curproc -> share == 0){
        // MLFQ scheduled process exits

        for (int i=0;i<=ptable.q_size[curproc->lev];i++){
            t = ptable.ARRAYQUEUE[curproc->lev][i];
            if(t == curproc){
                ptable.q_size[curproc->lev]--;
                for (int j=i;j<=ptable.q_size[curproc->lev];j++)
                    ptable.ARRAYQUEUE[curproc->lev][j] = ptable.ARRAYQUEUE[curproc->lev][j+1];
                break;
            }
        }
    }else{
        // STRIDE scheduled process exits
       
        for (int i=0;i<=stable.stable_size;i++){
        t = stable.proc[i];
        if(t == curproc){
            stable.stable_size--;
            for (int j=i;j<=stable.stable_size;j++)
                stable.proc[j] = stable.proc[j+1];
            break;
            }
        }
        stable.proc[0]->share += curproc->share;
    }
    //queue_table_lookup();

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


// only for process managed by MLFQ
void 
boost (void)
    {
    acquire(&ptable.lock);
    cprintf("boosting\n");

    struct proc* p;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    p->lev = 0;
    p->time_allotment = time_allotment[0];
    }
    release(&ptable.lock);
}


void
scheduler(void)
{
    struct proc *p;
    struct cpu *c = mycpu();
    c->proc = 0;

    time_quantom[STRIDE_LEV] = STRIDE_DEFAULT_TIME_QUANTOM;
    time_allotment[STRIDE_LEV] = STRIDE_DEFAULT_TIME_QUANTOM;

    // virtual process in STRIDE table
    // if the process is chosed, MLFQ scheduling is done.

    //cprintf("[Location] forkret() : %x\n",forkret);

    struct proc mlfq;
    mlfq.share = 100;
    mlfq.pass = 0;
    mlfq.pid = 85064538;
    mlfq.state = RUNNABLE;
    stable.proc[++stable.stable_size] = &mlfq;

    // prevent memory leakage
    int i,j;
    int min_index;

    for(;;){

    // Enable interrupts on this processor.
    // though kernel has control, Ctrl+A X has to terminated the system.
        sti();

        acquire(&ptable.lock);

        min_index = 0;
        for (i = 0; i <= stable.stable_size; i++ ){
            if(stable.proc[i]->state != RUNNABLE)
                continue;
            if(stable.proc[i] -> pass < stable.proc[min_index] -> pass){
                min_index = i;
            }
        }

        // even MLFQ's pass has to be added.
        p = stable.proc[min_index];
        p -> pass += MULTSTRIDESHARE / p->share;
        min_pass = p->pass;

        int found = 0;
        if(min_index == 0){
        // MLFQ scheduling
            for(i = 0; i < DEPTH_QUEUE; i++){
                for (j = 0; j <= ptable.q_size[i]; j++){
                    p = ptable.ARRAYQUEUE[i][j];
                    if(p->state != RUNNABLE)
                        continue;
                    if(p->share)
                        continue;
                    found = 1;
                    break;
                }
            if(found)
                break;
            }
        }

        // here RUNNABLE check is needed.
        // because there may no runnable process.

        if(p->state==RUNNABLE){
            if(p->first_scheduled == 0){
                p->first_scheduled = 1;
            }
            /*
            if(p->share==0)
            cprintf("[M%d]",p->pid);
            else
            cprintf("[S%d]",p->pid);
            */
            /*
            if(p->share==0){
            cprintf("lev[%d]",p->lev);
            for (int i=0;i<p->time_allotment;i++)
            cprintf("*");
            cprintf("\n");
            }
            */


            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.

            // start_tick is used for MLFQ but not matters 
            // even if we include it to STRIDE
            p->start_tick = MLFQ_ticks;

            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            swtch(&(c->scheduler), p->context);
            switchkvm();

            // Process is done running for now.
            // It should have changed its p->state before coming back.
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

void
thread_switch(thread* old, thread* new)
{
    acquire(&ptable.lock);
    old->state = RUNNABLE;
    release(&ptable.lock);
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
    if (myproc()->tid!=0)
        release(&ptable.lock);

    acquire(&ptable.lock);  //DOC: yieldlock
    myproc()->state = RUNNABLE;

    // adjust queue here
    // since p table cannot be accessed in trap.c
    // but able in proc.c here
    struct proc* p = myproc();

    // check if it came from MLFQ
    if(p->from_trap);
    else
        // malicious yield protection
        // 'yield' is giving cpu access up!
        // there is penalty, 
        // even if the process calling yield didn't use up 10ms for whole,
        // it is considered they have been used it totally!
        // time allotment is decremented.
        p->time_allotment--;

    if(p->share==0){

        // MLFQ
        int lev_pre = p->lev;

        if(p -> time_allotment == 0 
            && p -> lev != DEPTH_QUEUE - 1)
            p->lev++;      

        int lev_now = p->lev;

        // p->time_allotment = time_quantom[p->lev]; Intergrated with MLFQ below

        //dequeue in lev[lev_pre]
        for (int i=0;i<=ptable.q_size[lev_pre];i++){
            if(ptable.ARRAYQUEUE[lev_pre][i] == p){
                ptable.q_size[lev_pre]--;
            for (int j=i;j<=ptable.q_size[lev_pre];j++){
                ptable.ARRAYQUEUE[lev_pre][j] = ptable.ARRAYQUEUE[lev_pre][j+1];
            }
        }
    }

    //enqueue in lev[lev_now] as Last element.
    ptable.ARRAYQUEUE[lev_now][++ptable.q_size[lev_now]] = p;

    if(lev_now!=lev_pre)
        p -> time_allotment = time_allotment[p->lev];

    }else{
        // STRIDE
        // p -> time_allotment = time_quantom[p->lev]; Intergrated with MLFQ
        // its' level does not change.
        p -> time_allotment = time_allotment[p->lev];
    }


    sched();

    release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.


// [Thread] we cannot avoid forkret?
// I insisted to use forkret to release!
// but actually, not needed.
// need doing-nothing function maybe it would be better

void
do_nothing(void)
{
    cprintf("donothing\n");
}

void
forkret(void)
{
    /*
    if(myproc()){
        if((myproc() -> pid) == -1)
            cprintf("thread[%d] entering forkret()\n",myproc()->tid);
        else
            cprintf("process[%d] entering forkret()\n",myproc()->pid);
    }
    */
    if(myproc()->multi_threaded == 1 && myproc()->first_scheduled){
        myproc()->first_scheduled = 0;
        acquire(&ptable.lock);
    }
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

    /*
    if(myproc()){
        if((myproc() -> pid) == -1)
            cprintf("thread[%d] escaping forkret()\n",myproc()->tid);
        else
            cprintf("process[%d] escaping forkret()\n",myproc()->pid);
    }
    */

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
    if(p->state == SLEEPING && p->chan == chan)
    p->state = RUNNABLE;
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
    if(p->state == SLEEPING)
    p->state = RUNNABLE;
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
