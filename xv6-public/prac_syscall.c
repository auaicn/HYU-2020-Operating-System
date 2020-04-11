#include "types.h"
#include "defs.h"

// Simple system call
int
printk_str(char *str)
{
    cprintf("%s by printk_str\n",str);
    return 0xABCDABCD;
}

// wrapper function maybe..
int
sys_myfunction(void)
{
    char *str;
    
    if(argstr(0,&str)<0)
    {
        return -1;
    }
    
    return printk_str(str);
    
}