// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.
//
// The implementation uses two state flags internally:
// * B_VALID: the buffer data has been read from the disk.
// * B_DIRTY: the buffer data has been modified
//     and needs to be written to disk.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;

  // buffer cache는 30개짜리 깡배열.
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  // LRU가 나가리 된다.
  struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

//PAGEBREAK!
  // Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  // 30번을 하는데. 깡 배열을 돌면서 
  // 걔네의 next 는 head의 next 즉, head;
  // 걔네의 prev 는 헤드..
  // 머가 다른건데..?
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head;

    // 버퍼 마다 락이 있다. 이 블록이 data block이면, 동시 접근시 락으로 보호하기 위함.
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache.lock);

  // Is the block already cached?
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      // 현재 dev에 할당이 되어있으며 까지는 오케이
      // block no이 같냐는건.. 아. 여기서 잡히면
      // cache hit
      // ref 

      b->refcnt++;
      release(&bcache.lock);

      // 머.. 버퍼관련 락을 잡고 리턴해주고 싶은데,
      // sleep으로 기다리겠다. 그런 느낌인가봐
      acquiresleep(&b->lock);

      // return buffer
      return b;
    }
  }

  // cache miss
  // not dirty, 즉 clean page를 사용하겠다.
  // read중이면, ref가 0이 아닐텐데 배꿔주면,
  // 이상한걸 읽겠지. not desired action
  // 안전하게 clean 이면서 ref인걸 찾네.

  // Not cached; recycle an unused buffer.
  // Even if refcnt==0, B_DIRTY indicates a buffer is in use
  // because log.c has modified it but not yet committed it.
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
    if(b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->flags = 0;
      b->refcnt = 1;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // 그래도 없으면 panic
  // 없을수가 없나봐?
  
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
// block이 어떤건지 dev가 어던건지 아직 모르겠네 수업에 있나?
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if((b->flags & B_VALID) == 0) {
    iderw(b);
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  b->flags |= B_DIRTY;
  // dirty만 해주네. flush는 대체 어디서 되는거야?
  iderw(b);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  // bcache에다가 넣어주려고 락을 잡는구나 구조가 바뀌니까.
  acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // 중간에서 빼나보네
    // 아무튼 처음이나 마지막은 아닌걸로 보인다.
    b->next->prev = b->prev;
    b->prev->next = b->next;

    // 빼서 buffer cache에다가 넣어주는거 같지?
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  release(&bcache.lock);
}
//PAGEBREAK!
// Blank page.

