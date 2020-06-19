struct buf {
  int flags;
  // B_VALID & B_DIRTY & B_BUSY
  // but B_BUSY seems to be replaced by sleeplock

  uint dev;
  uint blockno;
  struct sleeplock lock; // for thread
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf *qnext; // disk queue
  uchar data[BSIZE];
};

// its size : 22 + 28 + 512 = 562 인가봐.

// flag에 관한 내용같네.
#define B_VALID 0x2  // buffer has been read from disk, the buffer contains copy of the block
#define B_DIRTY 0x4  // buffer needs to be written to disk

// there is B_BUSY flag, used in multh-threaded programming
