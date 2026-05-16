#include "syscall.h"
#include "user.h"

static const char child_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "child: fork grandchild and exit without wait\n";

static const char grandchild_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "grandchild: running after parent exit\n";

static const char grandchild_exit_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "grandchild: exit now\n";

static const char init_wait1_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "init: first wait done\n";

static const char init_wait2_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "init: second wait done\n";

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
        user_syscall(SYS_exit, 1, 0, 0);
    }
    else if (pid == 0)
    {
        /*
         * child 进程再 fork 出 grandchild。
         */
        long gpid = user_syscall(SYS_fork, 0, 0, 0);

        if (gpid < 0)
        {
            user_syscall(SYS_puts, (long)fork_fail_msg, 0, 0);
            user_syscall(SYS_exit, 1, 0, 0);
        }

        if (gpid == 0)
        {
            /*
             * grandchild 进程延迟一会儿再退出。
             * 这样 child 会先退出，grandchild 会变成孤儿进程。
             */
            user_syscall(SYS_puts, (long)grandchild_msg, 0, 0);
            user_delay();
            user_syscall(SYS_puts, (long)grandchild_exit_msg, 0, 0);
            user_syscall(SYS_exit, 0, 0, 0);
        }

        /*
         * child 不 wait grandchild，直接 exit。
         */
        user_syscall(SYS_puts, (long)child_msg, 0, 0);
        user_syscall(SYS_exit, 0, 0, 0);
    }

     /*
     * init 等两次：
     * 第一次回收 child；
     * 第二次回收被 reparent 到 init 的 grandchild。
     */
    user_syscall(SYS_wait, 0, 0, 0);
    user_syscall(SYS_puts, (long)init_wait1_msg, 0, 0);

    user_syscall(SYS_wait, 0, 0, 0);
    user_syscall(SYS_puts, (long)init_wait2_msg, 0, 0);

    user_syscall(SYS_exit, 0, 0, 0);

    for (;;) {}
}