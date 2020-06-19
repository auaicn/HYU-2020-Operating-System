// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};


#define NINDIRECT (BSIZE / sizeof(uint))
//#define MAXFILE (NDIRECT + NINDIRECT)
#define MAXFILE (NDIRECT + NINDIRECT + DINDIRECT_ARRAY_SIZE + TINDIRECT_ARRAY_SIZE)

//#define NDIRECT 12
#define NDIRECT 10
#define INDIRECT_IDX (10)
#define DINDIRECT_IDX (11)
#define TINDIRECT_IDX (12)
#define INDIRECT_ARRAY_SIZE (128)
#define DINDIRECT_ARRAY_SIZE (16384)
#define TINDIRECT_ARRAY_SIZE (2097152)


// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+3];   // Data block addresses
};

// Inodes per block.
// caculated to 8
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
// if inode number '68' is given, 
// inode_start + 1 block has inode numbered [8:16]
// '68' would be eighth block
// inode_start + 8 has [64:72]
// inode_start is 2.
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
// 
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

