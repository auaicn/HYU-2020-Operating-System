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

  if((fd = open("midtest.out",0x202)) < 0){
      printf(1, "midtest: cannot open %s\n", "midtest.out");
      exit();
  }

  //printf(1,"fd:%d\n",fd); 
  memmove(buf,"writing to midtest.out",30);

  if((n = write(fd,buf,512)) < 0)
    printf(1,"[proc]writing failed\n");
  
  printf(1,"HELLOWORLD\n");

  //fsync()
  //char sentence[512];
  /*
  close(fd);
  fd = open("midtest.out",0x002);
  if(read(fd,sentence,sizeof(sentence))<0){
	  printf(1,"read error\n");
  	exit();
  }

  printf(1,"sentence : %s\n",sentence);
  close(fd);
  //if((n = write(fd,buf,30)<0))
  */
    //printf(1,"[proc]writing failed\n");
  exit();
}
