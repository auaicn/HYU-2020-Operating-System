#include "types.h"
#include "stat.h"
#include "user.h"

//#define BIGSIZE (2097152)
#define BIGSIZE (102400)

char buf[BIGSIZE];
char read_sentence[BIGSIZE];
char big_file[BIGSIZE];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;

  if((fd = open("bigtest.out",0x202)) < 0){
      printf(1, "test: cannot open %s\n", "test.out");
      exit();
  }

  for (int i=0;i<BIGSIZE;i++)
    buf[i] = 'a';
  //printf(1,"fd:%d\n",fd); 
  memmove(buf,"writing to test.out",30);

  if((n = write(fd,buf,BIGSIZE)<0))
    printf(1,"[proc]writing failed\n");

  //fsync()
  //char read_sentence[BIGSIZE];
  close(fd);
  fd = open("bigtest.out",0x002);
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
