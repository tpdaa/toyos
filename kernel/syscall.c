#include "syscall.h"
#include "printf.h"
#include "vm.h"
#include "proc.h"
#include "fs.h"

static long sys_puts(uint64 uva)
{
    char buf[256];
    struct proc *p = myproc();

    if (p == 0 || p->pagetable == 0) {
        printf("sys_puts: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, buf, uva, sizeof(buf)) < 0) {
        printf("sys_puts: bad user string %p\n", (void *)uva);
        return -1;
    }
    
    printf("%s",buf);
    return 0;
}

static long sys_fork(void)
{
    return proc_fork();
}

static long sys_wait(void)
{
    return proc_wait();
}

static long sys_exec(uint64 program_id)
{
    return proc_exec((int)program_id);
}

static long sys_getprogid(void)
{
    return proc_get_program_id();
}

static long sys_readfile(uint64 uva, uint64 max)
{
    char kbuf[256];
    struct proc *p = myproc();
    int n;

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_readfile: no current process\n");
        return -1;
    }

    if (max == 0) 
    {
        return 0;
    }

    if (max > sizeof(kbuf)) 
    {
        max = sizeof(kbuf);
    }

    n = fs_readi(ROOTINO, kbuf, (unsigned int)max);
    if (n < 0) 
    {
        printf("sys_readfile: fs_readi failed\n");
        return -1;
    }

    if (copyout(p->pagetable, uva, kbuf, (uint64)n) < 0) 
    {
        printf("sys_readfile: copyout failed\n");
        return -1;
    }

    return n;
}

void syscall(struct trapframe *tf)
{
    uint64 num = tf->a7;

    //printf("syscall: num=%ld\n",num);

    switch(num)
    {
        case SYS_puts:
            tf->a0 = sys_puts(tf->a0);
            break;
        case SYS_exit:
            proc_exit((int)tf->a0);
            break;
        case SYS_yield:
            yield();
            tf->a0 = 0;
            break;
        case SYS_getpid:
        {
            struct proc *p = myproc();

            if (p == 0)
            {
                tf->a0 = -1;
            }
            else
            {
                tf->a0 = p->pid;
            }

            break;
        }
        case SYS_fork:
            tf->a0 = sys_fork();
            break;
        case SYS_wait:
            tf->a0 = sys_wait();
            break;
        case SYS_exec:
            tf->a0 = sys_exec(tf->a0);
            break;
        case SYS_getprogid:
            tf->a0 = sys_getprogid();
            break;
        case SYS_readfile:
            tf->a0 = sys_readfile(tf->a0, tf->a1);
            break;
        default:
            printf("unknown syscall: %ld\n",num);
            tf->a0 = (uint64)-1;
            break;
    }
}


