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

int time_quantom[10] = {5,10,20};
int time_allotment[10] = {20,40,0};

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
      MLFQ_ticks++;
      release(&tickslock);

    }

    if(! (MLFQ_ticks%PERIOD_BOOSTING) );
      //boost();

    // T interrupt has occured
    // global ticks variable incremented

    // MLFQ, STRIDE, single Threaded, MultiThreaded Process(Thread) have time quantom

    if(p->num_thread == 0){
      // Single Threaded
      p->time_allotment--; // anyway
      if( (p->time_allotment%time_quantom[p->lev])!=0 ){
        // Time LEFT
        return;
      }else
        // Time Finished 
        // yield will do extra jobs
        // MLFQ, Queue adjusting
        // STRIDE, time allotment resotration
        yield();
      
    }else
    {
      // Multi threaded
      // Yield only if in finishes it's time quantom. 
      p->time_allotment--; // anyway
      if( (p->time_allotment%time_quantom[p->lev])!=0 ){
        // Time LEFT
        // Return is not desirable
        // return;

        // Switching between LWPS
        struct proc* pp = p->parent;
        thread* nt = NULL;

        // Just Lookaside is OK?
        // acquire(&ptable.lock);

        for (int i=1;i<=pp->num_thread;i++){

          int idx = (p->tid+i)%pp->num_thread+1;
          cprintf("[%d/%d]\n",idx,pp->num_thread);
          nt = pp->threads[idx];
          
          if(nt->state!=RUNNABLE)
            continue;
          else{
            p->state = RUNNABLE;
            swtch(&p->context, nt->context);
            return;
          }
        }
        panic("thread to run not found");

      }else
        // Time Finished yield will do extra jobs
        // MLFQ, Queue adjusting
        // STRIDE, time allotment resotration

        // yield be careful
        // context switching between scheduler and 
        // but I think it would not make any issues.
        yield();

    }

  }

  // Check if the process has been killed since we yielded
  if(myproc() 
    && myproc()->killed 
    && (tf->cs&3) == DPL_USER)
    exit();
}

