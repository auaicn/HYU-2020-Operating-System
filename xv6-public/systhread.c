#include "types.h"
#include "defs.h"
#include "thread.h"

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