#include "types.h"
#include "stat.h"
#include "user.h"

#define BIGSIZE (10000)

char buf[BIGSIZE];
char sentence[BIGSIZE];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;

  // create and write

  if((fd = open("pread.out",0x202)) < 0){
      printf(1, "test: cannot open %s\n", "pread.out");
      exit();
  }

  for (int i=0;i<BIGSIZE;i++){
    buf[i] = 'a' + i%10;
  }

  if((n = write(fd,buf,BIGSIZE)) < 0)
    printf(1,"[proc]writing failed\n");

  close(fd);

  // read 
  fd = open("pread.out",0x002);

  printf(1, "\n10 8byte pread calls\n");
  for (int i=0;i<10;i++){
      read(fd, sentence, 8); sentence[8] = '\0';
      printf(1, "%s\n", sentence);
  }

  close(fd);

  // pread

  fd = open("pread.out",0x002);

  printf(1, "\n10 8byte pread calls. it's offset is 11, it's char will be b\n");
  for (int i=0;i<10;i++){
      pread(fd, sentence, 8, 11); sentence[8] = '\0';
      printf(1, "%s\n", sentence);
  }

  close(fd);

  // exit
  exit();  
  
}
