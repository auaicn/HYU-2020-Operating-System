#include "types.h"
#include "user.h"
#include "defs.h"

// 얘는 myfunction이라는 시스템콜을 내가 새로 만들었는데,
// 그걸 부르는 유저 프로그램이야.

int main(int argc,const char* argv[]){나

    char* buf = "Hello xv6!";
    int ret_val = printk_str(buf); 
    //myfunction where is it..?

    printf(1,"Return Value : 0x%x",ret_val);
    exit(0);
}
