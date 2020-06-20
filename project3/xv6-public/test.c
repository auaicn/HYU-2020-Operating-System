#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];
char sentence[512];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;

  if((fd = open("test.out",0x202)) < 0){
      printf(1, "test: cannot open %s\n", "test.out");
      exit();
  }

  //printf(1,"fd:%d\n",fd); 
  memmove(buf,"writing to test.out",30);

  if((n = write(fd,buf,512)) < 0)
    printf(1,"[proc]writing failed\n");
  
  printf(1,"HELLOWORLD\n");
}
