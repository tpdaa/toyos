#include "printf.h"
#include "trap.h"
#include "user.h"
#include "riscv.h"
#include "kalloc.h"
#include "vm.h"
#include "proc.h"
#include "timer.h"

void start(void)
{
    printf("Toyos kernel start.\n");

    trap_init();

    kinit();

    void  *p1 = kalloc();
    void  *p2 = kalloc();

    printf("kalloc test: p1=%p p2=%p\n", p1, p2);

    kfree(p1);

    void *p3 = kalloc();

    printf("kalloc test: after kfree(p1), p3=%p\n", p3);

    kfree(p2);
    kfree(p3);

    kvminit();
   // kvminithart();
    
    procinit();

    userinit();

    timer_init();
    
    scheduler();

    printf("ERROR: enter_user returned.\n");
    
    for(;;)
    {
        asm volatile("wfi");//wfi 是 RISC-V 的 “wait for interrupt” 指令，可以理解成“闲置等待”。
    }
}