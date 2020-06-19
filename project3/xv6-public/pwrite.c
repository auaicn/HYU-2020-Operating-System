#include "types.h"
#include "stat.h"
#include "user.h"

#define BIGSIZE (150)

char buf[BIGSIZE];
char sentence[BIGSIZE];
char result[BIGSIZE];

int 
main(int argc, char *argv[])
{
  int fd;
  int n;

  // array initialization
  for (int i=0;i<10;i++)
    sentence[i] = '0'+i;
  for (int i=0;i<BIGSIZE;i++){
    buf[i] = 'a' + i%10;
  }

  // create and write
  if((fd = open("pwrite.out",0x202)) < 0){
      printf(1, "test: cannot open %s\n", "pwrite.out");
      exit();
  }
  for (int i=0;i<BIGSIZE;i++){
    buf[i] = '-';
  }

  printf(1, "initial buf\n%s\n\n",buf);


  if((n = write(fd,buf,BIGSIZE)) < 0)
    printf(1,"[proc]writing failed\n");
  close(fd);

  // WRITE 10 times. offset 0, length 10.
  fd = open("pwrite.out",0x002);
  for (int i=0;i<10;i++)
      write(fd, sentence, 10);
  close(fd);

  // read result just read 1000
  fd = open("pwrite.out",0x002);
  printf(1, "write(fd,sentence,10) 10 times\n");
  if((n = read(fd,result,1000)) < 0 ){
    printf(1,"read error after write\n");
    return -1;
  }
  printf(1,"%s\n\n",result);
  close(fd);

  // create and write again.
  fd = open("pwrite.out",0x202);
  if((n = write(fd,buf,BIGSIZE)) < 0)
    printf(1,"[proc]writing failed\n");
  close(fd);

  // 'P' WRITE 10 times. offset 10, length 10
  fd = open("pwrite.out",0x002);
  printf(1, "pwrite(fd,sentence,10,10) 10 times\n");
  for (int i=0;i<10;i++){
      pwrite(fd, sentence, 10, 10);
  }
  close(fd);

  // read result just read 1000
  fd = open("pwrite.out",0x002);
  if((n = read(fd,result,1000)) < 0 ){
    printf(1,"read error after pwrite\n");
    return -1;
  }
  printf(1,"%s\n\n",result);
  close(fd);

  // exit
  exit();  
  
}
