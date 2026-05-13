#include "sbi.h"

int sbi_call(uint64 which,uint64 arg0,uint64 arg1,uint64 arg2)
{
    register uint64 a0 asm("a0") = arg0;//定义一个变量 a0，并强制让它绑定到 CPU 的 a0 寄存器，并赋值为 arg0
    register uint64 a1 asm("a1") = arg1;
    register uint64 a2 asm("a2") = arg2;
    register uint64 a7 asm("a7") = which;


    asm volatile(
        "ecall"
        :"=r"(a0)
        :"r"(a0),"r"(a1),"r"(a2),"r"(a7)
        :"memory"
    );

    return a0;
}

void sbi_console_putchar(int ch)
{
    sbi_call(1,ch,0,0);
}

void sbi_set_timer(uint64 stime_value)
{
    sbi_call(0, stime_value, 0, 0);
}