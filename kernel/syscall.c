#include "syscall.h"
#include "printf.h"
#include "vm.h"
#include "proc.h"

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
        default:
            printf("unknown syscall: %ld\n",num);
            tf->a0 = (uint64)-1;
            break;
    }
}
