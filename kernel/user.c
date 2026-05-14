#include "syscall.h"
#include "user.h"

static const char p1_before[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Process 1: before delay!\n";

static const char p1_after[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Process 1: after delay!\n";

static const char p2_before[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Process 2: before delay!\n";

static const char p2_after[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Process 2: after delay!\n";

static const char unknown_before[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Unknown process: before delay!\n";

static const char unknown_after[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "Unknown process: after delay!\n";


unsigned char user_stack[USER_STACK_SIZE]
    __attribute__((used, aligned(16), section(".user.bss")));

static long user_syscall(long num, long arg0, long arg1, long arg2)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long user_syscall(long num,long arg0, long arg1,long arg2)
{
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;
    register long a7 asm("a7") = num;

    asm volatile(
        "ecall"
        : "+r"(a0)
        :"r"(a1),"r"(a2),"r"(a7)
        :"memory"
    );

    return a0;
} 

static void user_delay(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void user_delay(void)
{
    for (volatile long i = 0; i < 100000000; i++) 
    {
    }
}

void user_main(void) __attribute__((used, noinline, aligned(16), section(".user.text")));

void user_main(void)
{
    long pid = user_syscall(SYS_getpid, 0, 0, 0);

    if (pid == 1)
    {
        user_syscall(SYS_puts, (long)p1_before, 0, 0);
    }
    else if (pid == 2)
    {
        user_syscall(SYS_puts, (long)p2_before, 0, 0);
    }
    else
    {
        user_syscall(SYS_puts, (long)unknown_before, 0, 0);
    }

    user_delay();

    if (pid == 1)
    {
        user_syscall(SYS_puts, (long)p1_after, 0, 0);
    }
    else if (pid == 2)
    {
        user_syscall(SYS_puts, (long)p2_after, 0, 0);
    }
    else
    {
        user_syscall(SYS_puts, (long)unknown_after, 0, 0);
    }

    user_syscall(SYS_exit, 0, 0, 0);

    for (;;) {}
}