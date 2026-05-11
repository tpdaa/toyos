#include "syscall.h"
#include "user.h"

static const char hello[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Hello from U-mode via syscall!\n";

unsigned char user_stack[USER_STACK_SIZE]
    __attribute__((used, aligned(16), section(".user.bss")));

static long user_syscall(long num, long arg0, long arg1, long arg2)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

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

void user_main(void) __attribute__((used, noinline, aligned(16), section(".user.text")));

void user_main(void)
{
    user_syscall(SYS_puts,(long)hello,0,0);
    user_syscall(SYS_exit,0,0,0);

    for(;;){}
}