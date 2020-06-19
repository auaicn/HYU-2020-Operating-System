#include "types.h"
#include "stat.h"
#include "user.h"

#define BIGSIZE (2097152)
#define TIMES (100)

char buf[BIGSIZE];
char read_sentence[BIGSIZE];
char big_file[BIGSIZE];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;
  int i;

  printf(1,"Stress test for big file (size:2097152) Start\n");
  for (i = 0;i < TIMES; i++){
    if((fd = open("bigtest.out",0x202)) < 0){
        printf(1, "test: cannot open %s\n", "test.out");
        exit();
    }

    char char_var = '1';
    for (int i=0;i<BIGSIZE;i++){
      if((i%512) == 0){
        if(char_var == '9')
          char_var = '0';
        else
          char_var ++;
      }
      buf[i] = char_var;
    }

    if((n = write(fd,buf,BIGSIZE)) < 0)
      printf(1,"[proc]writing failed\n");

    close(fd);
    printf(1,"Test[%d] Finished\n",i+1);
    unlink("bigtest.out");
  }
  printf(1,"Stress test for big file (size:2097152) Passed\n");

  /*
  fd = open("bigtest.out",0x002);
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
