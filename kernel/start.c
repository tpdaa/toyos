#include "sbi.h"

void start(void)
{
    const char *msg = "Hello World from ToyOS!\n";

    while(*msg)
    {
        sbi_putchar(*msg);
        msg++;
    }

    for(;;)
    {
        asm volatile("wfi");//wfi 是 RISC-V 的 “wait for interrupt” 指令，可以理解成“闲置等待”。
    }
}