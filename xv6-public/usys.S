#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)

SYSCALL(yield)
SYSCALL(set_cpu_share)
SYSCALL(getlev)

SYSCALL(queue_table_lookup)

SYSCALL(thread_create)
SYSCALL(thread_join)
SYSCALL(thread_exit)
SYSCALL(thread_self)
SYSCALL(thread_mutext_lock)
SYSCALL(thread_mutext_unlock)
SYSCALL(thread_mutext_init)
