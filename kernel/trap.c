#include "trap.h"
#include "riscv.h"
#include "printf.h"
#include "trapframe.h"
#include "syscall.h"

extern void trap_entry(void);

void trap_init(void)
{
    w_stvec((uint64)trap_entry);//设置入口
    printf("trap init done, stvec=%p\n",(void *)trap_entry);
}

void kernel_trap(struct trapframe *tf)
{
    uint64 scause = r_scause();
    uint64 sepc = r_sepc();
    uint64 stval = r_stval();

    printf("trap happened: scause=%lx sepc=%p stval=%lx\n",
            scause,(void *)sepc,stval);

    if(scause == 8)
    {
        w_sepc(sepc+4);
        syscall(tf);
        return;
    }
   
        
    if(scause==3)
    {
        printf("trapframe: tf=%p a0=%lx a1=%lx a7=%lx\n",
               (void *)tf, tf->a0, tf->a1, tf->a7);
        w_sepc(sepc+4);
        return;
    }

    printf("unhandled trap,hait.\n");

    for(;;)
    {
        asm volatile("wfi");
    }
}