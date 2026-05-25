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

static long sys_readfile(uint64 name_uva, uint64 buf_uva, uint64 max)
{
    char name[32];
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

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_readfile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    n = fs_readfile(name, kbuf, (unsigned int)max);
    if (n < 0) 
    {
        printf("sys_readfile: fs_readfile failed\n");
        return -1;
    }

    if (copyout(p->pagetable, buf_uva, kbuf, (uint64)n) < 0) 
    {
        printf("sys_readfile: copyout failed\n");
        return -1;
    }

    return n;
}

static long sys_listfiles(uint64 buf_uva, uint64 max)
{
    char kbuf[256];
    struct proc *p = myproc();
    int n;

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_listfiles: no current process\n");
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

    n = fs_list(kbuf, (unsigned int)max);
    if (n < 0) 
    {
        printf("sys_listfiles: fs_list failed\n");
        return -1;
    }

    if (copyout(p->pagetable, buf_uva, kbuf, (uint64)n) < 0) 
    {
        printf("sys_listfiles: copyout failed\n");
        return -1;
    }

    return n;
}

static long sys_createfile(uint64 name_uva, uint64 content_uva)
{
    char name[32];
    char content[256];
    struct proc *p = myproc();

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_createfile: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_createfile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    if (copystr(p->pagetable, content, content_uva, sizeof(content)) < 0) 
    {
        printf("sys_createfile: bad content %p\n", (void *)content_uva);
        return -1;
    }

    if (fs_create(name, content) < 0) 
    {
        printf("sys_createfile: fs_create failed\n");
        return -1;
    }

    return 0;
}

static long sys_statfile(uint64 name_uva, uint64 st_uva)
{
    char name[32];
    struct filestat st;
    struct proc *p = myproc();

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_statfile: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_statfile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    if (fs_stat(name, &st) < 0) 
    {
        printf("sys_statfile: fs_stat failed\n");
        return -1;
    }

    if (copyout(p->pagetable, st_uva, (const char *)&st, sizeof(st)) < 0) 
    {
        printf("sys_statfile: copyout failed\n");
        return -1;
    }

    return 0;
}

static long sys_unlinkfile(uint64 name_uva)
{
    char name[32];
    struct proc *p = myproc();

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_unlinkfile: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_unlinkfile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    if (fs_unlink(name) < 0) 
    {
        printf("sys_unlinkfile: fs_unlink failed\n");
        return -1;
    }

    return 0;
}

static long sys_writefile(uint64 name_uva, uint64 content_uva)
{
    char name[32];
    char content[256];
    struct proc *p = myproc();

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_writefile: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_writefile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    if (copystr(p->pagetable, content, content_uva, sizeof(content)) < 0) 
    {
        printf("sys_writefile: bad content %p\n", (void *)content_uva);
        return -1;
    }

    if (fs_writefile(name, content) < 0) 
    {
        printf("sys_writefile: fs_writefile failed\n");
        return -1;
    }

    return 0;
}

static long sys_appendfile(uint64 name_uva, uint64 content_uva)
{
    char name[32];
    char content[256];
    struct proc *p = myproc();

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_appendfile: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_appendfile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    if (copystr(p->pagetable, content, content_uva, sizeof(content)) < 0) 
    {
        printf("sys_appendfile: bad content %p\n", (void *)content_uva);
        return -1;
    }

    if (fs_appendfile(name, content) < 0) 
    {
        printf("sys_appendfile: fs_appendfile failed\n");
        return -1;
    }

    return 0;
}

static long sys_openfile(uint64 name_uva)
{
    char name[32];
    struct proc *p = myproc();
    int inum;
    int fd;

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_openfile: no current process\n");
        return -1;
    }

    if (copystr(p->pagetable, name, name_uva, sizeof(name)) < 0) 
    {
        printf("sys_openfile: bad filename %p\n", (void *)name_uva);
        return -1;
    }

    inum = fs_open(name);
    if (inum < 0) 
    {
        printf("sys_openfile: fs_open failed\n");
        return -1;
    }

    for (fd = 0; fd < NOFILE; fd++) 
    {
        if (!p->files[fd].used) 
        {
            p->files[fd].used = 1;
            p->files[fd].inum = (unsigned int)inum;
            p->files[fd].off = 0;
            return fd;
        }
    }

    printf("sys_openfile: no free fd\n");
    return -1;
}

static long sys_readfd(uint64 fd_arg, uint64 buf_uva, uint64 max)
{
    char kbuf[256];
    struct proc *p = myproc();
    int fd = (int)fd_arg;
    int n;

    if (p == 0 || p->pagetable == 0) 
    {
        printf("sys_readfd: no current process\n");
        return -1;
    }

    if (fd < 0 || fd >= NOFILE || !p->files[fd].used) 
    {
        printf("sys_readfd: bad fd=%d\n", fd);
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

    n = fs_readi_at(p->files[fd].inum, kbuf, (unsigned int)max, p->files[fd].off);
    if (n < 0) 
    {
        printf("sys_readfd: fs_readi_at failed\n");
        return -1;
    }

    if (copyout(p->pagetable, buf_uva, kbuf, (uint64)n) < 0) 
    {
        printf("sys_readfd: copyout failed\n");
        return -1;
    }

    p->files[fd].off += (unsigned int)n;

    return n;
}

static long sys_closefd(uint64 fd_arg)
{
    struct proc *p = myproc();
    int fd = (int)fd_arg;

    if (p == 0) 
    {
        return -1;
    }

    if (fd < 0 || fd >= NOFILE || !p->files[fd].used) 
    {
        printf("sys_closefd: bad fd=%d\n", fd);
        return -1;
    }

    p->files[fd].used = 0;
    p->files[fd].inum = 0;
    p->files[fd].off = 0;

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
            tf->a0 = sys_readfile(tf->a0, tf->a1, tf->a2);
            break;
        case SYS_listfiles:
            tf->a0 = sys_listfiles(tf->a0, tf->a1);
            break;
        case SYS_createfile:
            tf->a0 = sys_createfile(tf->a0, tf->a1);
            break;
        case SYS_statfile:
            tf->a0 = sys_statfile(tf->a0, tf->a1);
            break;
        case SYS_unlinkfile:
            tf->a0 = sys_unlinkfile(tf->a0);
            break;
        case SYS_openfile:
            tf->a0 = sys_openfile(tf->a0);
            break;
        case SYS_readfd:
            tf->a0 = sys_readfd(tf->a0, tf->a1, tf->a2);
            break;
        case SYS_closefd:
            tf->a0 = sys_closefd(tf->a0);
            break;
        case SYS_writefile:
            tf->a0 = sys_writefile(tf->a0, tf->a1);
            break;
        case SYS_appendfile:
            tf->a0 = sys_appendfile(tf->a0, tf->a1);
            break;
        default:
            printf("unknown syscall: %ld\n",num);
            tf->a0 = (uint64)-1;
            break;
    }
}


