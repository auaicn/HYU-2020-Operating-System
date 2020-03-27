#include "types.h"
#include "defs.h"

// Simple system call
int
printk_str(char *str)
{
    cprintf("%s\n",str);
    return 0xABCDABCD;
}