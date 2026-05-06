#include "trap.h"
#include "riscv.h"
#include "printf.h"

extern void trap_entry(void);

void trap_init(void)
{
    w_stvec((uint64)trap_entry);//设置入口
    printf("trap init done, stvec=%p\n",(void *)trap_entry);
}

void kernel_trap(void)
{
    uint64 scause = r_scause();
    uint64 sepc = r_sepc();
    uint64 stval = r_stval();

    printf("trap happened: scause=%lx sepc=%p stval=%lx\n",
            scause,(void *)sepc,stval);

    if(scause==3)
    {
        w_sepc(sepc+4);
        return;
    }

    printf("unhandled trap,hait.\n");

    for(;;)
    {
        asm volatile("wfi");
    }
}