#include "types.h"
#include "stat.h"
#include "user.h"

#define BIGSIZE (100000)

char buf[BIGSIZE];
char read_sentence[BIGSIZE];
char big_file[BIGSIZE];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;

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

  if((fd = open("bigtest.out",0x202)) < 0){
      printf(1, "test: cannot open %s\n", "test.out");
      exit();
  }

  for (int i=0;i<100;i++){
    printf(1, "[%d]th 1,000,000 write start \n",i+1);
    if((n = write(fd,buf,BIGSIZE)) < 0)
      printf(1,"[proc]writing failed\n");
  }

  close(fd);
  exit();  
  
}
