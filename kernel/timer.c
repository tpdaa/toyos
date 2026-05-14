#include "timer.h"
#include "riscv.h"
#include "sbi.h"
#include "printf.h"

/*
 * QEMU virt 里 OpenSBI 输出过：
 * Timer Device : aclint-mtimer @ 10000000Hz
 *
 * 也就是 time 寄存器大约每秒增加 10000000。
 * 这里先设置成 1000000，大约 0.1 秒一次。
 */
#define TIMER_INTERVAL 1000000UL

#define TIME_SLICE 1

static uint64 ticks = 0;
static int slice_ticks = 0;

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
    slice_ticks = 0;

    timer_set_next();
    /*
     * 开启 S-mode timer interrupt。
     * SIE_STIE 是 Supervisor Timer Interrupt Enable。
     */
    w_sie(r_sie() | SIE_STIE);

    /*
     * 开启 S-mode 全局中断。
     */
    //w_sstatus(r_sstatus() | SSTATUS_SIE);

    printf("timer init done.\n");
}

int timer_tick(void)
{
    ticks++;
    slice_ticks++;
    
    timer_set_next();

    //每 TIME_SLICE 个 timer tick 触发一次调度。
    if (slice_ticks >= TIME_SLICE)
    {
        slice_ticks = 0;
        return 1;
    }

    return 0;

}

uint64 timer_ticks(void)
{
    return ticks;
}