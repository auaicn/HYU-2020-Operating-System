// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

void
bootmain(void)
{
  struct elfhdr *elf; // get kernel image
  struct proghdr *ph, *eph; // page header maybe
  void (*entry)(void); // 생김새는 내가아는거랑 아주 똑같네. assembly 부르는게 아니라 그런가? 아닌가?
  uchar* pa;

  elf = (struct elfhdr*)0x10000;  // scratch space

  // Read 1st page off disk
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  // Load each program segment (ignores ph flags).
  // phoff : page header offset
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);

  // eph : end of page header
  eph = ph + elf->phnum;

  // which not reach end of page header
  // page header may be 32bit. ph++ 을 보면 하나씩 증가시켜도 충분히 다
  // 정보를 가지고 올 수 있나보다.
  // 아냐 struct type casting 되서 1씩, 4씩 오르는게 아니라
  // 크기만큼 충분하게 커지는거야.
  // 또 아냐. 얘는 포인터라서 4바이트인게 맞아 ->를 하니까 계산을 해서 읽어서 값을 가져오고 
  // read segment 에 넣어주는거야.

  for(; ph < eph; ph++){

    // ph 에서 page addr 랑 filesz 부분을 갖와서 readsegment
    pa = (uchar*)ph->paddr;
    readseg(pa, ph->filesz, ph->off);
    if(ph->memsz > ph->filesz)
      // if memory size is bigger than file size
      // remaining part cleared to 0 byte by byte.
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }

  // Call the entry point from the ELF header.
  // Does not return!

  entry = (void(*)(void))(elf->entry);

  // call
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
void
readsect(void *dst, uint offset)
{
  // 아무튼 이 파일부터는 음.. 32bit x86인건가?
  // insl outb 는 device communication 을 위한 legacy in out instruction 비슷한거같은데
  // 맞네 들어가보니까 assembly로 port에 넣고 빼네.
  // disk 도 device니깐 거기서 읽어오고 그러는거구나

  // Issue command.
  waitdisk();

  // outb(ushort port, uchar data)
  // 번대 port에 각각의 정보를 보내고
  // port마다 8bit 밖에 못보내나봐 4번에 나눠서 
  // 보내고 1은 signal같다. Nope 아래 0x1F7 로 0x20 시그널이 read이다
  // signal 중에 0x20이 read signal 같네.

  outb(0x1F2, 1);   // count = 1

  // giving sector number
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);

  // read comment.
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  // wait what? disk!!
  waitdisk();

  // 496번 포트에서 dst로 받나보네.
  // #define SECTSIZE  512
  // 32비트 단위여서 4로 나누어서 받나보다. 128번 받겠다는거같네.
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' 
// from kernel !!! into physical address 'pa'.
// Might copy more than asked.
void
readseg(uchar* pa, uint count, uint offset)
{
  //cprintf("%x %x %x\n",pa,count,offset);
  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
