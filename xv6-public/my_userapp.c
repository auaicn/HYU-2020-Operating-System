#include "types.h"
#include "user.h"
#include "stat.h"

int main(int argc,const char* argv[]){
    char* buf = "Hello xv6!";
    int ret_val;
    ret_val = myfunction(buf);
    printf(1,"Return Value : 0x%x",ret_val);
    return 0;
}
