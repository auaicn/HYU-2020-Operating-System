#include "types.h"
#include "defs.h"

int printk_str(char* str)
{
	cprintf("%s\n",str);
	return 0xABCDABCD;
}
