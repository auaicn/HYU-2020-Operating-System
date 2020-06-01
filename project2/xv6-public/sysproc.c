#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

/*
int 
sys_thread_self(void)
{
  return thread_self();
}

int 
sys_thread_mutex_lock(void)
{
  return thread_mutex_lock;
}

int 
sys_thread_mutex_unlock(void)
{
  return thread_mutex_unlock;
}

int 
sys_thread_mutex_init(void)
{
  return thread_mutex_init;
}
*/

int 
sys_thread_create(void)
{
  thread_t *thread_;
  void* (*start_rountine_)(void*);
  void *arg_;

  if(argptr(0, (void*)&thread_,sizeof(*thread_)) < 0          ||
    argptr(1, (void*)&start_rountine_,sizeof(*start_rountine_)) < 0   ||
    argptr(2, (void*)&arg_,sizeof(*arg_)) < 0)
    return -1;

  return thread_create(thread_, start_rountine_, arg_);
}

int 
sys_thread_exit(void)
{
  void *retval_;
  if(argint(0, (int*)&retval_) < 0)
    return -1;

  thread_exit(retval_);
  return 0;
}

int
sys_thread_join(void)
{
  thread_t thread_;
  void **retval_;

  if(argint(0, (int*)&thread_) < 0
    ||argint(1, (int*)&retval_) < 0)
    return -1;

  return thread_join(thread_, retval_);
}

// ThrEAD

//below is my implementation

int
sys_queue_table_lookup(void)
{
  queue_table_lookup();
  return 0; //success
}


int 
sys_yield(void)
{
	myproc()->from_trap = 0;
	yield();
	return 0;
}

//above is my implementation
int
sys_getlev(void)
{
  return getlev();
}

int
sys_set_cpu_share(void)
{
  int share_;
  if(argint(0, &share_) < 0)
    return -1;
  return set_cpu_share(share_)
;}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_getppid(void)
{
	return myproc()->parent->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
