#include "syscall.h"
#include "user.h"

static const char parent_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "parent: fork returned child pid\n";

static const char child_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "child: fork returned 0\n";

static const char fork_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fork failed\n";


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
    long pid = user_syscall(SYS_fork, 0, 0, 0);

    if (pid < 0)
    {
        user_syscall(SYS_puts, (long)fork_fail_msg, 0, 0);
    }
    else if (pid == 0)
    {
        user_syscall(SYS_puts, (long)child_msg, 0, 0);
    }
    else
    {
        user_syscall(SYS_puts, (long)parent_msg, 0, 0);
    }

    user_syscall(SYS_exit, 0, 0, 0);

    for (;;) {}
}