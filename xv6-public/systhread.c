#include "types.h"
#include "defs.h"
#include "thread.h"

// thread.c
int 
sys_thread_create(void)
{
  thread_t *thread_;
  void* (*start_rountine_)(void*);
  void *arg_;

  if(argint(0, &thread_) < 0          ||
    argint(1, &start_rountine_) < 0   ||
    argint(2, &arg_) < 0)
    return -1;

  return sys_thread_create(thread_t *thread_, void* (*start_rountine_)(void*), void *arg_);
}

void 
sys_thread_exit(void)
{
  void *retval_;
  if(argint(0, &retval_) < 0)
    return -1;

  return sys_thread_exit(void *retval_);
}

int
sys_thread_join(thread_t thread, void **retval)
{
 	struct thread_t thread_;
  void **retval_;

  if(argint(0, &thread_) < 0
    ||argint(1, &retval_) < 0)
    return -1;

  return sys_thread_join(thread_t thread_, void **retval_);
}
