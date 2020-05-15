#include "types.h"
#include "defs.h"
#include "thread.h"

// thread.c
int 
sys_thread_create(thread_t *thread, void* (*start_rountine)(void*), void *arg)
{
  struct thread_t *thread_;
  void* (*start_rountine)(void*)
  void *arg
  int major, minor;

  begin_op();
  if((argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
	thread_t
}

void 
sys_thread_exit(void *retval)
{
	void* retval;
	(argint(0,&retval))

}

int
sys_thread_join(thread_t thread, void **retval)
{
 	
}
