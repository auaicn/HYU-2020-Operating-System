#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

// device도 file로 취급하는구나.
// stat은 file status라고 생각하면 되겠다.
// nlink는 나중에 파일을 아예 닫을지 말지 확인할때 0인지 확인하겠네.
// size는 많이 참조되는 부분이니깐 여 있나봐.

struct stat {
  short type;  // Type of file 위에 3가지중 하나.
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
};
