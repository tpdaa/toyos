#include "printf.h"

void start(void)
{
    printf("Hello World from ToyOS!\n");

    printf("num=%d hex=%x str=%s ptr=%p\n", 123, 123, "ok", (void *)start);
    
    for(;;)
    {
        asm volatile("wfi");//wfi 是 RISC-V 的 “wait for interrupt” 指令，可以理解成“闲置等待”。
    }
};