#include "timer.h"
#include "riscv.h"
#include "sbi.h"
#include "printf.h"
#include "proc.h"

/*
 * QEMU virt 里 OpenSBI 输出过：
 * Timer Device : aclint-mtimer @ 10000000Hz
 *
 * 也就是 time 寄存器大约每秒增加 10000000。
 * 这里先设置成 1000000，大约 0.1 秒一次。
 */
#define TIMER_INTERVAL 1000000UL

static uint64 ticks;

static uint64 r_time(void)
{
    uint64 x;

    asm volatile("rdtime %0" : "=r"(x));

    return x;
}

static void timer_set_next(void)
{
    uint64 now = r_time();

    sbi_set_timer(now + TIMER_INTERVAL);
}

void timer_init(void)
{
    ticks = 0;

    /*
     * 开启 S-mode timer interrupt。
     * SIE_STIE 是 Supervisor Timer Interrupt Enable。
     */
    w_sie(r_sie() | SIE_STIE);

    /*
     * 开启 S-mode 全局中断。
     */
    //w_sstatus(r_sstatus() | SSTATUS_SIE);

    timer_set_next();

    printf("timer init done.\n");
}

void timer_tick(void)
{
    struct proc *p = myproc();

    ticks++;

    if ((ticks % 1) == 0)
    {
        if (p != 0)
        {
            printf("timer interrupt: ticks=%lx pid=%d\n", ticks, p->pid);
        }
        else
        {
            printf("timer interrupt: ticks=%lx pid=none\n", ticks);
        }
    }

    timer_set_next();
}

uint64 timer_ticks(void)
{
    return ticks;
}