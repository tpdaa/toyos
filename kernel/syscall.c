#include "syscall.h"
#include "printf.h"
#include "vm.h"

static long sys_puts(uint64 uva)
{
    char buf[256];

    if(copystr(user_pagetable, buf ,uva, sizeof(buf)) < 0)
    {
        printf("sys_puts: bad user string %p\n", (void *)uva);
        return -1;
    }
    
    printf("%s",buf);
    return 0;
}

static long sys_exit(long code)
{
    printf("user exit, code=%ld\n",code);

    for(;;)
    {
        asm volatile("wfi");
    }
    return 0;
}

void syscall(struct trapframe *tf)
{
    uint64 num = tf->a7;

    printf("syscall: num=%ld\n",num);

    switch(num)
    {
        case SYS_puts:
            tf->a0 = sys_puts(tf->a0);
            break;
        case SYS_exit:
            tf->a0 = sys_exit((long)tf->a0);
            break;
        default:
            printf("unknown syscall: %ld\n",num);
            tf->a0 = (uint64)-1;
            break;
    }
}
