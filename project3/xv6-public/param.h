#define NPROC        64  // maximum number of processes
#define NTHREAD      64  // maximum number of threads
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log // 30 block
#define MAXOPBLOCKS  40  // max # of blocks any FS op writes
//#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache // 30개 밖에 안되나
#define FSSIZE       10000000  // size of file system in blocks
#ifndef NULL
#define NULL		 ((void*)0)
#endif

