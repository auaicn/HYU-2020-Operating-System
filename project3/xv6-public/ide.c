// Simple PIO-based (non-DMA) IDE driver code.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

#define SECTOR_SIZE   512
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;
static struct buf *idequeue;

static int havedisk1;
static void idestart(struct buf*);

// Wait for IDE disk to become ready.
static int
idewait(int checkerr)
{
  int r;

  while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
    ;
  if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
    return -1;
  return 0;
}

void
ideinit(void)
{
  int i;

  initlock(&idelock, "ide");

  // multiprocessor
  ioapicenable(IRQ_IDE, ncpu - 1);
  idewait(0);

  // Check if disk 1 is present
  outb(0x1f6, 0xe0 | (1<<4));
  for(i=0; i<1000; i++){
    if(inb(0x1f7) != 0){
      havedisk1 = 1;
      break;
    }
  }

  // Switch back to disk 0.
  outb(0x1f6, 0xe0 | (0<<4));
}

// Start the request for b.  Caller must hold idelock.
static void
idestart(struct buf *b)
{
  if(b == 0)
    panic("idestart");
  if(b->blockno >= FSSIZE){
    cprintf("ide.c: block no : %d\n",b->blockno);
    panic("incorrect blockno");
  }
  int sector_per_block =  BSIZE/SECTOR_SIZE; // becomes 1 here.
  int sector = b->blockno * sector_per_block; // sector equals to block no here.
  int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
  int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

  if (sector_per_block > 7) panic("idestart");

  idewait(0);
  outb(0x3f6, 0);  // generate interrupt
  outb(0x1f2, sector_per_block);  // number of sectors
  outb(0x1f3, sector & 0xff);
  outb(0x1f4, (sector >> 8) & 0xff);
  outb(0x1f5, (sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
  if(b->flags & B_DIRTY){
    outb(0x1f7, write_cmd);
    outsl(0x1f0, b->data, BSIZE/4);
  } else {
    outb(0x1f7, read_cmd);
  }
}

// Interrupt handler.
void
ideintr(void)
{
  struct buf *b;

  // First queued buffer is the active request.
  acquire(&idelock);

  if((b = idequeue) == 0){
    release(&idelock);
    return;
  }
  idequeue = b->qnext;

  // Read data if needed.
  if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
    insl(0x1f0, b->data, BSIZE/4);

  // Wake process waiting for this buf.
  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;
  wakeup(b);

  // Start disk on next buf in queue.
  if(idequeue != 0){
    cprintf("in ideintr\n");
    idestart(idequeue);
  }

  release(&idelock);
}

//PAGEBREAK!
// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
// 여깄네!! 여기 sync buf with disk라고 하네
// bwrite가 불렸으면, B_DIRTY가 설정이 되거든.

// bread 랑 bwrite만 여기를 부르네
// bwrite 만이 dirty set 하구.
// disk driver read write
void
iderw(struct buf *b)
{
  struct buf **pp;

  if(!holdingsleep(&b->lock))
    panic("iderw: buf not locked");

  // valid but not dirty
  // 근데 dirty여야 여기를 들어오도록 했나봐. 그건 
  // bwrite에서는 그냥 dirty set하고 불렀는데?
  // valid는 좀더 예전에 걸러지나봐.
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
     // should not have been called and entered here.
     panic("iderw: nothing to do");

  // dev 는 device 이다.
  // havedisk1는 전역으로 0으로 되어 있는데,
  // boot 시 main.c:main():ideinit()에서 inb(0x1f7) 이 0이 아닌 값이면 1로 set이 되네.
  // ideinit()에서 advanced programmable interrupt controller 의 traps.h:IRQ_IDE (14) 가 ncpu -1 번 cpu에 한해 enable 되네
  // npcu = 1 지금 CPUS=1 로 돌리니까 현재 있는 하나의 프로세서에 interrupt enable 해주는거 같네.
  // 잘되는지 idewait(0) 해서 확인해준거 같고. 이제 disk 1이 머 어디 쓰이는진 모르겠지만, fs.img 이려나!!
  // 
  
  /*
  x86.h:
  static inline void
  outb(ushort port, uchar data)
  {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
  }
  아 인자 두개구나. 10000 | 0xe0 dmf 0x1f6 port로 보내고 그러네.
  보냈으니 이제 신호를 기다리는부분이 inb(0x1f7) 이다. 
  b 는 byte를 뜻해서, 8bit bus인지 아무튼 1000번을 읽어오네.
  하나라도 0이 아닌 값이 들어오면 disk1이 있다고 설정을 해주고, 더이상 inb를 하지 않고 break 후
  disk 0으로 돌아간대. 돌아가는게 outb로 되는진 모르겟지만.
  */


  // 아무튼 이 함수는 말이야. havedisk1이 1이어야지 동작을 한다.
  // buffer를 쓰든 읽어올라든 그 출처가, 대상이 필요할거 아냐
  // buffer 의 device 정보는 0이어야 하고, 다시 한번 말하지만, disk 가 있어야 작동을 한다.
  if(b->dev != 0 && !havedisk1)
    panic("iderw: ide disk 1 not present");

  acquire(&idelock);  //DOC:acquire-lock

  // Append b to idequeue.
  // append 니까 정말 마지막에 붙이네. MRU랬나?
  // last element가 될거니깐, 
  b->qnext = 0;
  for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
    ;
  *pp = b;

  // 하나 넣었는데, 그놈이 그놈이야. 그럼 비어있었다는 얘기.
  // Start disk if necessary.
  
  if(idequeue == b){
    cprintf("in ide read/write iderw block no [%d] it consists of char [%d] pid [%d]\n",b->blockno,b->data[0]-'0',myproc()->pid);
    idestart(b);
  }

  // Oh it waits! then surely written I suppose
  // Wait for request to finish
  // maybe, kernel(myproc() == 0) executer would handle all in once.
  // because sleep() internally uses ptable lock
  // we want the buf on queue
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
    sleep(b, &idelock);
  }


  release(&idelock);
}
