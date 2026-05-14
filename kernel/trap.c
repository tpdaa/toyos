#include "trap.h"
#include "riscv.h"
#include "printf.h"
#include "trapframe.h"
#include "syscall.h"
#include "proc.h"
#include "timer.h"

extern void trap_entry(void);

void trap_init(void)
{
    w_stvec((uint64)trap_entry);//设置入口

    w_sscratch(0);

    printf("trap init done, stvec=%p\n",
            (void *)trap_entry);
}

static void copy_trapframe(struct trapframe *dst, struct trapframe *src)
{
    uint64 *d = (uint64 *)dst;
    uint64 *s = (uint64 *)src;

    for (int i = 0; i < sizeof(struct trapframe) / sizeof(uint64); i++) {
        d[i] = s[i];
    }
}

void kernel_trap(struct trapframe *tf)
{
    struct trapframe *stack_tf = tf;
    struct proc *p = myproc();

    uint64 scause = r_scause();
    uint64 sepc = r_sepc();
    uint64 stval = r_stval();

    int from_user = (tf->sp != 0);

    if (from_user && p != 0) 
    {
        copy_trapframe(&p->trapframe, tf);
        tf = &p->trapframe;
    }

    printf("trap happened: scause=%lx sepc=%p stval=%lx\n",
            scause,(void *)sepc,stval);

    if ((scause & SCAUSE_INTERRUPT) && ((scause & ~SCAUSE_INTERRUPT) == SCAUSE_TIMER))
    {
        timer_tick();

        if (from_user && p != 0 && p->state == RUNNING)
        {
            yield();
        }

        w_sepc(sepc);

        if (from_user && p != 0) 
        {
            copy_trapframe(stack_tf, &p->trapframe);
        }

        return;
    }
    
    if(scause == 8)
    {
        syscall(tf);
        w_sepc(sepc + 4);

        if (from_user && p != 0) 
        {
           copy_trapframe(stack_tf, &p->trapframe);
        }

        return;
    }
    
    if(scause==3)
    {
        printf("trapframe: tf=%p a0=%lx a1=%lx a7=%lx\n",
               (void *)tf, tf->a0, tf->a1, tf->a7);
        w_sepc(sepc+4);

        if (from_user && p != 0) 
        {
            copy_trapframe(stack_tf, &p->trapframe);
        }

        return;
    }

    printf("unhandled trap,halt.\n");

    for(;;)
    {
        asm volatile("wfi");
    }
}