#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];
char read_sentence[512];

#define BIFSIZE (2097152)

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

  if((n = write(fd,buf,512)<0))
    printf(1,"[proc]writing failed\n");

  //fsync()
  //char read_sentence[512];
  close(fd);
  fd = open("test.out",0x002);
  if(read(fd,read_sentence,sizeof(read_sentence))<0){
	  printf(1,"read error\n");
  	exit();
  }

  printf(1,"read_sentence : %s\n",read_sentence);
  close(fd);
  //if((n = write(fd,buf,30)<0))
    //printf(1,"[proc]writing failed\n");
  return 0;
}
