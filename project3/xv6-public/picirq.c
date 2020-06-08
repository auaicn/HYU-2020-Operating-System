#include "types.h"
#include "x86.h"
#include "traps.h"

// I/O Addresses of the two programmable interrupt controllers
#define IO_PIC1         0x20    // Master (IRQs 0-7)
#define IO_PIC2         0xA0    // Slave (IRQs 8-15)

// Don't use the 8259A interrupt controllers.  Xv6 assumes SMP hardware.
void
picinit(void)
{
  // mask all interrupts
  outb(IO_PIC1+1, 0xFF); // 0x20 이면 32다. T interrupt 인것 같네. // Timer 1 set to 1 and then enabled 부분은 어디일까
  outb(IO_PIC2+1, 0xFF);
}

//PAGEBREAK!
// Blank page.
