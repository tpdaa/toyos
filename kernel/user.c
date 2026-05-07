#include "syscall.h"

static long user_syscall(long num,long arg0, long arg1,long arg2)
{
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;
    register long a7 asm("a7") = num;

    asm volatile(
        "ecall"
        : "+r"(a0)
        :"r"(a1),"r"(a2),"r"(a7)
        :"memory"
    );

    return a0;
} 

void user_main(void)
{
    user_syscall(SYS_puts,(long)"Hello from U-mode via syscall!\n",0,0);
    user_syscall(SYS_exit,0,0,0);

    for(;;){}
}