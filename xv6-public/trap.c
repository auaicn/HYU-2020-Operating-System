#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;

uint ticks;
uint MLFQ_ticks;

#ifndef STRIDE_LEV
#define STRIDE_LEV (9)
#endif

#ifndef STRIDE_DEFAULT_TIME_QUANTOM
#define STRIDE_DEFAULT_TIME_QUANTOM (5)
#endif

#ifndef PERIOD_BOOSTING
#define PERIOD_BOOSTING (200)
#endif

#ifndef DEPTH_QUEUE
#define DEPTH_QUEUE (3)
#endif

int time_quantom[10] = {5,10,20};
int time_allotment[10] = {20,40,0};

#ifndef ptable_struct
struct ptable_struct{
struct spinlock lock;
struct proc proc[NPROC];
struct proc* ARRAYQUEUE[DEPTH_QUEUE][NPROC];
int q_size[DEPTH_QUEUE];
};
#endif

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void 
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() 
    && myproc()->state == RUNNING 
    && tf->trapno == T_IRQ0+IRQ_TIMER){

    struct proc* p = myproc();

    // Boosting needs pre emption.
    if(p->share == 0){
      // MLFQ scheduled Single or MultiThreaded Process
      // MLFQ_ticks is MLFQ scheduling methos' exclusive Property
      acquire(&tickslock);
      //cprintf("MLFQ ticks:[%d]",MLFQ_ticks);
      MLFQ_ticks++;
      release(&tickslock);

    }

    // shell or init process is also managed by MLFQ and shares MLFQ_ticks.
    // but init and sh doest not frequently grab it.
    // when CPUS=1 qemu-nox is executed,
    // after $ (prompt) shown to us, sh go to sleep. and no process is runnable at that time.
    // before we enter the xv6 command, during the period, always scheduler(kernel)
    // is handling cpu.
    if(! (MLFQ_ticks%PERIOD_BOOSTING) )
      boost();

    // T interrupt has occured
    // global ticks variable incremented

    // MLFQ, STRIDE, single Threaded, MultiThreaded Process(Thread) have time quantom

    if(p -> multi_threaded == 0){
      // Single Threaded

      // time alootment decreased
      p -> time_allotment--; 

      /*
      //if(p -> share == 0){
        cprintf("pid[%d]lev[%d]",p->pid,p->lev);
        for (int i=0;i<p->time_allotment;i++)
          cprintf("*");
        cprintf("\n");
      }
      */  

      if(( p->time_allotment % time_quantom[p->lev] ) != 0){
        // Time LEFT
        // do not sched
        return;
      }else{
        // alloted time quantom Finished 
        // yield will do extra jobs
        // MLFQ, Queue adjusting
        // STRIDE, time allotment resotration
        p->from_trap = 1;
        yield();
      }
      
    }else {
      return;
      
      yield();
      return;
      // Multi threaded
      // Yield only if in finishes it's time quantom. 
      cprintf("\nT interrupt while multi threaded program pid%d tid[%d]\n",myproc()->pid,myproc()->tid);

      thread* mthread = p -> master_thread;
      mthread->time_allotment--;

      if( (mthread -> time_allotment % time_quantom[p->lev])!=0 ){

        // Time quantom left
        // Switching between LWPS
        thread* next_th = NULL;
        
        // what yield does with lock
        acquire(&(ptable.lock));
        p -> state = RUNNABLE; 
        // release(&ptable.lock);

        for (int i=0;i<=mthread->num_thread;i++){

            // idx for round robin implementation            
            int idx = (p->tid+(i+1))%(mthread -> num_thread + 1);
            next_th = mthread->threads[idx];
            // cprintf("runnable%d running%d sleeping%d\n",RUNNABLE,RUNNING,SLEEPING);
            // cprintf("%d, state %d \n",idx,next_th->state);

            if(next_th->state!=RUNNABLE)
                continue;

            // thread to run found
            // ptable lock needed but not accessible here
            // 1. import ptable with extern keyword 
            //    and make some acquire release wrapper function..
            // 2. make thread state. but need locking needed.. maybe
            //    because many threads can look through it.
            // 3. Not implement locking
            //    it's critical section but thread table lookup is only done here
            //    also thread-thread communication not needed for now!

            // what scheduler does before swtching
            // no access to ptable lock but I think it's not needed.
            // although, state trasition may need it.
           //  cprintf("myproc : %d %d\n", myproc()->pid,myproc()->tid);

            // acquire(&(ptable.lock));
            mycpu()->proc = next_th;
            next_th -> state = RUNNING;
            release(&(ptable.lock));

            // we cannot avoid forkret?
            // I insisted to use forkret to release!
            // but actually, not needed.
            // need doing-nothing function maybe it would be better
            cprintf("tid[%d -> %d] giving up cpu\n",p->tid, next_th->tid);

            /*
            int intena;
            intena = mycpu()->intena;

            if(p->state == RUNNING)
                panic("sched running");
            if(readeflags()&FL_IF)
                panic("sched interruptible");
            if(mycpu()->ncli != 1)
                panic("sched locks");
            */
            
            swtch(&(p->context), next_th->context);
            //mycpu()->intena = intena;

            cprintf("pid[%d]tid[%d] retrieved cpu\n",next_th->pid,next_th->tid);
            cprintf("time to return\n");
            return;
        }

        // thread to run not found
        // with sleep implementation maybe later can be?
        panic("thread to run not found");

      }else{
        cprintf("time allot finished. yield from trap\n");

        // Time Finished yield will do extra jobs
        // MLFQ, Queue adjusting
        // STRIDE, time allotment resotration

        // yield be careful
        // context switching between scheduler and 
        // but I think it would not make any issues.
        p->from_trap = 1;

        // do not directly switching from working thread to 
        // swtch(&(p->context),p->parent->context);
        //if (myproc()->tid!=0)
          //release(&ptable.lock);

        yield();
      }
    }

  }

  // Check if the process has been killed since we yielded
  if(myproc() 
    && myproc()->killed 
    && (tf->cs&3) == DPL_USER)
    exit();
}

