#include "syscall.h"
#include "user.h"

static const char shell_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "\nToyOS shell start\n";

static const char shell_run_hello_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run hello\n";

static const char shell_run_count_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run count\n";

static const char shell_run_cat_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run cat\n";

static const char shell_run_ls_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run ls\n";

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

static const char cat_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "cat: reading toyfs\n";

static const char cat_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "cat: readfile failed\n";

static const char cat_missing_bad_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "cat: unexpected success reading missing.txt\n";

static const char cat_missing_ok_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "cat: missing.txt not found, good\n";

static const char cat_filename[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "hello.txt";

static const char cat_readme_filename[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "readme.txt";

static const char cat_missing_filename[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "missing.txt";

static const char ls_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "ls: root directory\n";

static const char ls_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "ls: listfiles failed\n";
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

static long uputs(const char *s)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long uputs(const char *s)
{
    return user_syscall(SYS_puts, (long)s, 0, 0);
}

static void uexit(int code)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void uexit(int code)
{
    user_syscall(SYS_exit, code, 0, 0);
    for (;;) {}
}

static long ufork(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long ufork(void)
{
    return user_syscall(SYS_fork, 0, 0, 0);
}

static long uwait(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long uwait(void)
{
    return user_syscall(SYS_wait, 0, 0, 0);
}

static long uexec(int program_id)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long uexec(int program_id)
{
    return user_syscall(SYS_exec, program_id, 0, 0);
}

static long ugetprogid(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long ugetprogid(void)
{
    return user_syscall(SYS_getprogid, 0, 0, 0);
}

static long readfile(const char *name, char *buf, unsigned long max)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long readfile(const char *name, char *buf, unsigned long max)
{
    return user_syscall(SYS_readfile, (long)name, (long)buf, (long)max);
}

static long listfiles(char *buf, unsigned long max)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long listfiles(char *buf, unsigned long max)
{
    return user_syscall(SYS_listfiles, (long)buf, (long)max, 0);
}

static void hello_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void hello_main(void)
{
    uputs(hello_msg);
    uexit(0);

    for (;;) {}
}

static void count_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void count_main(void)
{
    uputs(count1_msg);
    uputs(count2_msg);
    uputs(count3_msg);
    uexit(0);

    for (;;) {}
}

static void cat_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void cat_main(void)
{
    char buf[128];
    long n;

    uputs(cat_start_msg);

    n = readfile(cat_filename, buf, sizeof(buf) - 1);

    if (n < 0) 
    {
        uputs(cat_fail_msg);
        uexit(1);
    }

    buf[n] = '\0';
    uputs(buf);

    n = readfile(cat_readme_filename, buf, sizeof(buf) - 1);
    if (n < 0) 
    {
        uputs(cat_fail_msg);
        uexit(1);
    }

    buf[n] = '\0';
    uputs(buf);

        n = readfile(cat_missing_filename, buf, sizeof(buf) - 1);
    if (n < 0) 
    {
        uputs(cat_missing_ok_msg);
    } else 
    {
        buf[n] = '\0';
        uputs(cat_missing_bad_msg);
        uputs(buf);
        uexit(1);
    }

    uexit(0);

    for (;;) {}
}

static void ls_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void ls_main(void)
{
    char buf[128];
    long n;

    uputs(ls_start_msg);

    n = listfiles(buf, sizeof(buf) - 1);
    if (n < 0) {
        uputs(ls_fail_msg);
        uexit(1);
    }

    buf[n] = '\0';
    uputs(buf);

    uexit(0);

    for (;;) {}
}

static void shell_run(const char *msg, int program_id)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void shell_run(const char *msg, int program_id)
{
    uputs(msg);

    long pid = ufork();

    if (pid < 0)
    {
        uputs(fork_fail_msg);
        return;
    }

    if (pid == 0)
    {
        uputs(before_exec_msg);
        uexec(program_id);

        /*
         * exec 成功后不应该返回旧代码。
         */
        uputs(exec_fail_msg);
        uexit(1);
    }

    uwait();
    uputs(shell_wait_msg);
}

static void shell_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void shell_main(void)
{
    uputs(shell_start_msg);

    shell_run(shell_run_hello_msg, PROG_HELLO);
    shell_run(shell_run_count_msg, PROG_COUNT);
    shell_run(shell_run_cat_msg, PROG_CAT);
    shell_run(shell_run_ls_msg, PROG_LS);

    uputs(shell_done_msg);
    uexit(0);

    for (;;) {}
}

void user_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

void user_main(void)
{
    long prog = ugetprogid();

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
    else if (prog == PROG_CAT)
    {
        cat_main();
    }
    else if (prog == PROG_LS)
    {
        ls_main();
    }
    else
    {
        uputs(unknown_prog_msg);
        uexit(1);
    }

    for (;;) {}
}
