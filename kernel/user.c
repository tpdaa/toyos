#include "syscall.h"
#include "user.h"

struct ufilestat {
    unsigned int inum;
    unsigned int type;
    unsigned int size;
    unsigned int data_block;
};

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

static const char shell_run_create_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run create\n";

static const char shell_run_stat_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run stat\n";

static const char shell_run_unlink_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run unlink\n";

static const char shell_run_write_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run write\n";

static const char shell_run_append_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run append\n";

static const char shell_run_fdtest_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run fdtest\n";

static const char shell_run_fdwrite_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "shell: run fdwrite\n";

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

static const char cat_note_filename[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "note.txt";

static const char cat_user_filename[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "user.txt";

static const char ls_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "ls: root directory\n";

static const char ls_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "ls: listfiles failed\n";

static const char create_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "create: creating user.txt\n";

static const char create_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "create: done\n";

static const char create_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "create: failed\n";

static const char create_filename[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "user.txt";

static const char create_content[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "created from user mode\n";

static const char create_dup_ok_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "create: duplicate user.txt rejected, good\n";

static const char create_dup_bad_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "create: unexpected duplicate create success\n";

static const char stat_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "stat: files\n";

static const char stat_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "stat: failed\n";

static const char unlink_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unlink: removing user.txt\n";

static const char unlink_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unlink: done\n";

static const char unlink_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unlink: failed\n";

static const char unlink_read_ok_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unlink: user.txt read failed after unlink, good\n";

static const char unlink_read_bad_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unlink: unexpected read success after unlink\n";

static const char write_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "write: overwrite note.txt\n";

static const char write_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "write: done\n";

static const char write_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "write: failed\n";

static const char write_content[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "updated by writefile from user mode\n";

static const char append_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "append: append note.txt\n";

static const char append_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "append: done\n";

static const char append_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "append: failed\n";

static const char append_content[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "append line from user mode\n";

static const char fdtest_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fdtest: open/read/close\n";

static const char fdtest_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fdtest: done\n";

static const char fdtest_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fdtest: failed\n";

static const char fdwrite_start_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fdwrite: open/write/close note.txt\n";

static const char fdwrite_done_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fdwrite: done\n";

static const char fdwrite_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fdwrite: failed\n";

static const char fdwrite_content[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "written by fd write\n";

static const char fork_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "fork failed\n";

static const char exec_fail_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "exec failed or returned to old code\n";

static const char unknown_prog_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "unknown program id\n";

static const char stat_prefix_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "stat: ";

static const char stat_inum_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    " inum=";

static const char stat_size_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    " size=";

static const char stat_block_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    " block=";

static const char newline_msg[] __attribute__((used, aligned(16), section(".user.rodata"))) =
    "\n";

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

static long createfile(const char *name, const char *content)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long createfile(const char *name, const char *content)
{
    return user_syscall(SYS_createfile, (long)name, (long)content, 0);
}

static long statfile(const char *name, struct ufilestat *st)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long statfile(const char *name, struct ufilestat *st)
{
    return user_syscall(SYS_statfile, (long)name, (long)st, 0);
}

static long unlinkfile(const char *name)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long unlinkfile(const char *name)
{
    return user_syscall(SYS_unlinkfile, (long)name, 0, 0);
}

static long writefile(const char *name, const char *content)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long writefile(const char *name, const char *content)
{
    return user_syscall(SYS_writefile, (long)name, (long)content, 0);
}

static long appendfile(const char *name, const char *content)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long appendfile(const char *name, const char *content)
{
    return user_syscall(SYS_appendfile, (long)name, (long)content, 0);
}

static long openfile(const char *name)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long openfile(const char *name)
{
    return user_syscall(SYS_openfile, (long)name, 0, 0);
}

static long readfd(long fd, char *buf, unsigned long max)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long readfd(long fd, char *buf, unsigned long max)
{
    return user_syscall(SYS_readfd, fd, (long)buf, (long)max);
}

static long writefd(long fd, const char *buf, unsigned long n)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long writefd(long fd, const char *buf, unsigned long n)
{
    return user_syscall(SYS_writefd, fd, (long)buf, (long)n);
}

static long closefd(long fd)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static long closefd(long fd)
{
    return user_syscall(SYS_closefd, fd, 0, 0);
}

static void uitoa(unsigned int x, char *buf)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void uitoa(unsigned int x, char *buf)
{
    char tmp[16];
    int i = 0;
    int j = 0;

    if (x == 0) 
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (x > 0) 
    {
        tmp[i++] = '0' + (x % 10);
        x /= 10;
    }

    while (i > 0) 
    {
        buf[j++] = tmp[--i];
    }

    buf[j] = '\0';
}

static void print_stat_line(const char *name, struct ufilestat *st)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void print_stat_line(const char *name, struct ufilestat *st)
{
    char num[16];

    uputs(stat_prefix_msg);
    uputs(name);

    uputs(stat_inum_msg);
    uitoa(st->inum, num);
    uputs(num);

    uputs(stat_size_msg);
    uitoa(st->size, num);
    uputs(num);

    uputs(stat_block_msg);
    uitoa(st->data_block, num);
    uputs(num);

    uputs(newline_msg);
}

static unsigned long ustrlen(const char *s)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static unsigned long ustrlen(const char *s)
{
    unsigned long n = 0;

    while (s[n] != '\0') 
    {
        n++;
    }

    return n;
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

    n = readfile(cat_note_filename, buf, sizeof(buf) - 1);
    if (n < 0) 
    {
        uputs(cat_fail_msg);
        uexit(1);
    }

    buf[n] = '\0';
    uputs(buf);

    n = readfile(cat_user_filename, buf, sizeof(buf) - 1);
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

static void create_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void create_main(void)
{
    long r;

    uputs(create_start_msg);

    r = createfile(create_filename, create_content);
    if (r < 0) 
    {
        uputs(create_fail_msg);
        uexit(1);
    }

    uputs(create_done_msg);

    /*
     * Test duplicate create path.
     * Creating the same file again should fail.
     */
    r = createfile(create_filename, create_content);
    if (r < 0) 
    {
        uputs(create_dup_ok_msg);
    } 
    else 
    {
        uputs(create_dup_bad_msg);
        uexit(1);
    }
    uexit(0);

    for (;;) {}
}

static void stat_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void stat_main(void)
{
    struct ufilestat st;

    uputs(stat_start_msg);

    if (statfile(cat_filename, &st) < 0) 
    {
        uputs(stat_fail_msg);
        uexit(1);
    }
    print_stat_line(cat_filename, &st);

    if (statfile(cat_readme_filename, &st) < 0) 
    {
        uputs(stat_fail_msg);
        uexit(1);
    }
    print_stat_line(cat_readme_filename, &st);

    if (statfile(cat_note_filename, &st) < 0) 
    {
        uputs(stat_fail_msg);
        uexit(1);
    }
    print_stat_line(cat_note_filename, &st);

    if (statfile(cat_user_filename, &st) < 0) 
    {
        uputs(stat_fail_msg);
        uexit(1);
    }
    print_stat_line(cat_user_filename, &st);

    uexit(0);

    for (;;) {}
}

static void unlink_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void unlink_main(void)
{
    char buf[128];
    long r;

    uputs(unlink_start_msg);

    r = unlinkfile(cat_user_filename);
    if (r < 0) 
    {
        uputs(unlink_fail_msg);
        uexit(1);
    }

    uputs(unlink_done_msg);

    /*
     * 删除后再读 user.txt，应该失败。
     */
    r = readfile(cat_user_filename, buf, sizeof(buf) - 1);
    if (r < 0) 
    {
        uputs(unlink_read_ok_msg);
    } 
    else 
    {
        buf[r] = '\0';
        uputs(unlink_read_bad_msg);
        uputs(buf);
        uexit(1);
    }

    uexit(0);

    for (;;) {}
}

static void write_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void write_main(void)
{
    long r;

    uputs(write_start_msg);

    r = writefile(cat_note_filename, write_content);
    if (r < 0) 
    {
        uputs(write_fail_msg);
        uexit(1);
    }

    uputs(write_done_msg);
    uexit(0);

    for (;;) {}
}

static void append_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void append_main(void)
{
    long r;

    uputs(append_start_msg);

    r = appendfile(cat_note_filename, append_content);
    if (r < 0) 
    {
        uputs(append_fail_msg);
        uexit(1);
    }

    uputs(append_done_msg);
    uexit(0);

    for (;;) {}
}

static void fdtest_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void fdtest_main(void)
{
    char buf[128];
    long fd;
    long n;

    uputs(fdtest_start_msg);

    fd = openfile(cat_filename);
    if (fd < 0) 
    {
        uputs(fdtest_fail_msg);
        uexit(1);
    }

    n = readfd(fd, buf, sizeof(buf) - 1);
    if (n < 0) 
    {
        uputs(fdtest_fail_msg);
        uexit(1);
    }

    buf[n] = '\0';
    uputs(buf);

    if (closefd(fd) < 0) 
    {
        uputs(fdtest_fail_msg);
        uexit(1);
    }

    uputs(fdtest_done_msg);
    uexit(0);

    for (;;) {}
}

static void fdwrite_main(void)
    __attribute__((used, noinline, aligned(16), section(".user.text")));

static void fdwrite_main(void)
{
    long fd;
    long n;

    uputs(fdwrite_start_msg);

    fd = openfile(cat_note_filename);
    if (fd < 0) 
    {
        uputs(fdwrite_fail_msg);
        uexit(1);
    }

    n = writefd(fd, fdwrite_content, ustrlen(fdwrite_content));
    if (n < 0) 
    {
        uputs(fdwrite_fail_msg);
        closefd(fd);
        uexit(1);
    }

    if (closefd(fd) < 0) 
    {
        uputs(fdwrite_fail_msg);
        uexit(1);
    }

    uputs(fdwrite_done_msg);
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
    shell_run(shell_run_create_msg, PROG_CREATE);
    shell_run(shell_run_write_msg, PROG_WRITE);
    shell_run(shell_run_append_msg, PROG_APPEND);
    shell_run(shell_run_fdwrite_msg, PROG_FDWRITE);
    shell_run(shell_run_cat_msg, PROG_CAT);
    shell_run(shell_run_ls_msg, PROG_LS);
    shell_run(shell_run_stat_msg, PROG_STAT);
    shell_run(shell_run_fdtest_msg, PROG_FDTEST);
    shell_run(shell_run_unlink_msg, PROG_UNLINK);
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
    else if (prog == PROG_CREATE)
    {
        create_main();
    }
    else if (prog == PROG_STAT)
    {
        stat_main();
    }
    else if (prog == PROG_UNLINK)
    {
        unlink_main();
    }
    else if (prog == PROG_FDTEST)
    {
        fdtest_main();
    }
    else if (prog == PROG_WRITE)
    {
        write_main();
    }
    else if (prog == PROG_APPEND)
    {
        append_main();
    }
    else if (prog == PROG_FDWRITE)
    {
        fdwrite_main();
    }
    else
    {
        uputs(unknown_prog_msg);
        uexit(1);
    }

    for (;;) {}
}
