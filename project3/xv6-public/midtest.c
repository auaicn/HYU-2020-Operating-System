#include "types.h"
#include "stat.h"
#include "user.h"

//#define BIGSIZE (2097152)
#define BIGSIZE (1000000)

char buf[BIGSIZE];
char read_sentence[BIGSIZE];
char big_file[BIGSIZE];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;

  if((fd = open("midtest.out",0x202)) < 0){
      printf(1, "midtest: cannot open %s\n", "midtest.out");
      exit();
  }

  char char_var = '1';
  // make buffer.
  for (int i=0;i<BIGSIZE;i++){
    if((i%512) == 511){
      if(char_var == '9')
        char_var = '0';
      else
        char_var ++;
      buf[i] = '\n';
    }
    else
      buf[i] = char_var;
  }
  //printf(1,"fd:%d\n",fd); 
  //memmove(buf,"writing to test.out",30);

  if((n = write(fd,buf,BIGSIZE)) < 0)
    printf(1,"[proc]writing failed\n");
  printf(1,"HELLOWORLD!\n");

  //fsync()
  //char read_sentence[BIGSIZE];
  close(fd);
  /*
  fd = open("midtest.out",0x002);
  if(read(fd,read_sentence,sizeof(read_sentence))<0){
	  printf(1,"read error\n");
  	exit();
  }

  printf(1,"read_sentence : %s\n",read_sentence);
  close(fd);
  //if((n = write(fd,buf,30)<0))
    //printf(1,"[proc]writing failed\n");
  */
  exit();
}
