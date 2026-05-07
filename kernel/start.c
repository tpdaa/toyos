#include "printf.h"
#include "trap.h"
#include "user.h"
#include "riscv.h"

static unsigned char user_stack[16384] __attribute__((aligned(16)));

void start(void)
{
    printf("Toyos kernel start.\n");

    trap_init();

    printf("enter user mode...\n");

    enter_user((uint64)user_main,
                (uint64)(user_stack + sizeof(user_stack)));
   
    printf("ERROR: enter_user returned.\n");
    
    for(;;)
    {
        asm volatile("wfi");//wfi 是 RISC-V 的 “wait for interrupt” 指令，可以理解成“闲置等待”。
    }
};