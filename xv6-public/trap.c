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
uint total_ticks;

int time_quantom[3] = {1,2,4};

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
      //cprintf("in Timer\n");
     
      // ticks now only related to mlfq.
      // actually stride is managed by pass(STRIDE SCHEDULER)
      // actually stride scheduler is not related with ticks.
      // but not adding ticks in stride case can be cause some..
      // sleep with timer? but there's no time I think
      // and then sleep() paramenter is chan(channel), which is
      // address of to_wake_up process
      
      // queue implementation here ? maybe think more.
      
      // maybe overhead but better than return at scheduler explicitly I think
      // maybe p could be 0 (kernel, scheduler) when interrupted while executing scheduler code
      // scheduler has context and share could be initialized to 0
      // then how to handle it ?

      /*
      struct proc* p = myproc();
      cprintf("AUAICN\n");
      cprintf("0x%x\n",p);
      cprintf("pid(%d) state(%d) share(%d)\n",p->pid,p->state,p->share);
      if(p->share == 0){
        acquire(&tickslock);
        total_ticks++;
        ticks++;
        wakeup(&ticks);
        release(&tickslock);

        // not have idea what below 1 line represents but..
        // needed or need not ?
        //lapiceoi();
        if(p -> start_tick + time_quantom[p->lev] <= ticks){
          // time to be changed.
          // yield  implemented below 
          //        not explicitly call here.
          // start_tick   has changed in proc.c:scheduler()
          //              right before context switch is applied.
          if(p->lev==2);
            //do nothing
          else{
            p->age--;
            if(p->age);
            else{
              p -> lev++;
              p -> age = 5;
            }
          }
        }
        else{
          // time quantom guaranteed here
          // yield is similar to "IO bound jobs" compared to "compute jobs"
          // should guarantee it's execution more.
          // just return
          yield();
          //return;
        }
      }
    }
    */
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
    /*
    cprintf("AUAICN\n");
    cprintf("0x%x\n",p);
    cprintf("pid(%d) state(%d) share(%d)\n",p->pid,p->state,p->share);
    */
    if(p->share == 0){
      
      acquire(&tickslock);
      total_ticks++;
      release(&tickslock);
      /*
      acquire(&tickslock);
      total_ticks++;
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
      */

      // not have idea what below 1 line represents but..
      // needed or need not ?
      //lapiceoi();
      if(p -> start_tick + time_quantom[p->lev] <= total_ticks){
        // time to be changed.
        // yield  implemented below 
        //        not explicitly call here.
        // start_tick   has changed in proc.c:scheduler()
        //              right before context switch is applied.
        if(p->lev==2)
          return;
            //do nothing
        else{
          p->age--;
          if(p->age);
          else{
            p -> lev++;
            p -> age = 5;
          }
        } 
      }else
        return;
      // time quantom guaranteed here
      // yield is similar to "IO bound jobs" compared to "compute jobs"
      // should guarantee it's execution more.
      // just return
    }
    yield();
  } 
  

  // Check if the process has been killed since we yielded
  if(myproc() 
    && myproc()->killed 
    && (tf->cs&3) == DPL_USER)
    exit();
}
