#include "syscall.h"
#include "printf.h"

static long sys_puts(const char *s)
{
    if(s == 0)
    {    return -1;}
    
        printf("%s",s);
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
            tf->a0 = sys_puts((const char *)tf->a0);
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
