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

     /*
     * Set several registers before ebreak,
     * so trapframe can prove it saved them correctly.
     *
     * a0 = 0x111
     * a1 = 0x222
     * a7 = 0x333
     */
    asm volatile(
        "li a0, 0x111\n"
        "li a1, 0x222\n"
        "li a7, 0x333\n"
        ".4byte 0x00100073\n"
        :
        :
        : "a0", "a1", "a7", "memory"
    );

    printf("After trap\n");
    
    for(;;)
    {
        asm volatile("wfi");//wfi 是 RISC-V 的 “wait for interrupt” 指令，可以理解成“闲置等待”。
    }
};