#include "syscall.h"
#include "user.h"

static const char shell_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "\nToyOS shell start\n";

static const char shell_run_hello_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run hello\n";

static const char shell_run_count_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run count\n";

static const char shell_wait_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: command finished\n";

static const char shell_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: all commands done\n";

static const char before_exec_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "child: before exec\n";

static const char hello_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "hello: running from PROG_HELLO\n";

static const char count1_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "count: 1\n";

static const char count2_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "count: 2\n";

static const char count3_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "count: 3\n";

static const char fork_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fork failed\n";

static const char exec_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "exec failed or returned to old code\n";

static const char unknown_prog_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unknown program id\n";

static void user_delay(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

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

static void shell_run_command(const char *cmd_msg)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void shell_run_command(const char *cmd_msg)
{
    user_syscall(SYS_puts, (long)cmd_msg, 0, 0);

    long pid = user_syscall(SYS_fork, 0, 0, 0);

    if (pid < 0)
    {
        user_syscall(SYS_puts, (long)fork_fail_msg, 0, 0);
        return;
    }

    if (pid == 0)
    {
        user_syscall(SYS_puts, (long)before_exec_msg, 0, 0);

        /*
         * 子进程执行 exec。
         * 当前 exec 是简化版，会重新加载同一个 user image。
         */
        user_syscall(SYS_exec, 0, 0, 0);

        /*
         * 如果 exec 成功，不应该执行到这里。
         */
        user_syscall(SYS_puts, (long)exec_fail_msg, 0, 0);
        user_syscall(SYS_exit, 1, 0, 0);
    }

    /*
     * shell 等待 command 子进程结束。
     */
    user_syscall(SYS_wait, 0, 0, 0);
    user_syscall(SYS_puts, (long)shell_wait_msg, 0, 0);
}

static void hello_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void hello_main(void)
{
    user_syscall(SYS_puts, (long)hello_msg, 0, 0);
    user_syscall(SYS_exit, 0, 0, 0);

    for (;;) {}
}

static void count_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void count_main(void)
{
    user_syscall(SYS_puts, (long)count1_msg, 0, 0);
    user_syscall(SYS_puts, (long)count2_msg, 0, 0);
    user_syscall(SYS_puts, (long)count3_msg, 0, 0);
    user_syscall(SYS_exit, 0, 0, 0);

    for (;;) {}
}

static void shell_run(const char *msg, int program_id)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void shell_run(const char *msg, int program_id)
{
    user_syscall(SYS_puts, (long)msg, 0, 0);

    long pid = user_syscall(SYS_fork, 0, 0, 0);

    if (pid < 0)
    {
        user_syscall(SYS_puts, (long)fork_fail_msg, 0, 0);
        return;
    }

    if (pid == 0)
    {
        user_syscall(SYS_puts, (long)before_exec_msg, 0, 0);
        user_syscall(SYS_exec, program_id, 0, 0);

        /*
         * exec 成功后不应该返回旧代码。
         */
        user_syscall(SYS_puts, (long)exec_fail_msg, 0, 0);
        user_syscall(SYS_exit, 1, 0, 0);
    }

    user_syscall(SYS_wait, 0, 0, 0);
    user_syscall(SYS_puts, (long)shell_wait_msg, 0, 0);
}

static void shell_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void shell_main(void)
{
    user_syscall(SYS_puts, (long)shell_start_msg, 0, 0);

    shell_run(shell_run_hello_msg, PROG_HELLO);
    shell_run(shell_run_count_msg, PROG_COUNT);

    user_syscall(SYS_puts, (long)shell_done_msg, 0, 0);
    user_syscall(SYS_exit, 0, 0, 0);

    for (;;) {}
}

void user_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

void user_main(void)
{
    long prog = user_syscall(SYS_getprogid, 0, 0, 0);

    if (prog == PROG_SHELL)
    {
        shell_main();
    }
    else if (prog == PROG_HELLO)
    {
        hello_main();
    }
    else if (prog == PROG_COUNT)
    {
        count_main();
    }
    else
    {
        user_syscall(SYS_puts, (long)unknown_prog_msg, 0, 0);
        user_syscall(SYS_exit, 1, 0, 0);
    }

    for (;;) {}
}
