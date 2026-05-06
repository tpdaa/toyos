#include "printf.h"
#include "trap.h"

void start(void)
{
    printf("Toyos kernel start.\n");

    trap_init();

    printf("Before trap\n");

    /*
     * 0x00100073 is the 32-bit encoding of ebreak.
     * It should trigger a breakpoint exception.
     */

    asm volatile(".4byte 0x00100073");

    printf("After trap\n");
    
    for(;;)
    {
        asm volatile("wfi");//wfi 是 RISC-V 的 “wait for interrupt” 指令，可以理解成“闲置等待”。
    }
};