#include "proc.h"
#include "printf.h"

static struct proc proc[NPROC];

struct proc *initproc;

static struct proc *current_proc;

static struct context scheduler_context;

static int nextpid = 1;

extern void enter_user(uint64 entry, uint64 sp);
extern void swtch(struct context *old, struct context *new);

static void forkret(void);

static void safestrcpy(char *dst, const char *src, int n)
{
    int i;

    if (n <= 0) 
    {
        return;
    }

    for (i = 0; i < n - 1 && src[i] != '\0'; i++) 
    {
        dst[i] = src[i];
    }

    dst[i] = '\0';
}

static void memset_bytes(void *dst, int value, uint64 n)
{
    unsigned char *p = (unsigned char *)dst;

    for (uint64 i = 0; i < n; i++) 
    {
        p[i] = value;
    }
}

uint64 proc_kstack_top(struct proc *p)
{
    return (uint64)(p->kstack + KSTACK_SIZE);
}

void procinit(void)
{
    for (int i = 0; i < NPROC; i++) 
    {
        proc[i].pid = 0;
        proc[i].state = UNUSED;
        proc[i].pagetable = 0;
        proc[i].entry = 0;
        proc[i].stack_top = 0;
        memset_bytes(&proc[i].trapframe, 0, sizeof(proc[i].trapframe));
        memset_bytes(&proc[i].context, 0, sizeof(proc[i].context));
        memset_bytes(proc[i].kstack, 0, KSTACK_SIZE);
        proc[i].name[0] = '\0';
    }

    initproc = 0;
    current_proc = 0;
    memset_bytes(&scheduler_context, 0, sizeof(scheduler_context));

    printf("proc init done.\n");
}

static struct proc *allocproc(void)
{
    for (int i = 0; i < NPROC; i++) 
    {
        if (proc[i].state == UNUSED) 
        {
            struct proc *p = &proc[i];

            p->pid = nextpid++;
            p->state = USED;
            p->pagetable = 0;
            p->entry = 0;
            p->stack_top = 0;

            memset_bytes(&p->trapframe, 0, sizeof(p->trapframe));
            memset_bytes(&p->context, 0, sizeof(p->context));
            memset_bytes(p->kstack, 0, KSTACK_SIZE);

             /*
             * 关键：
             * 这个进程第一次被 scheduler 通过 swtch() 切过去时，
             * swtch 会恢复 p->context.ra，然后 ret。
             *
             * 因为 ra = forkret，所以 ret 后会进入 forkret()。
             */
            p->context.ra = (uint64)forkret;

            /*
             * 这个进程第一次运行内核代码时，使用自己的内核栈。
             */
            p->context.sp = proc_kstack_top(p);

            safestrcpy(p->name, "init", sizeof(p->name));

            return p;
        }
    }

    return 0;
}

struct proc *userinit(void)
{
    struct proc *first = 0;

    for (int i = 0; i < NPROC; i++) 
    {
        struct proc *p = allocproc();

        if (p == 0) 
        {
            printf("userinit: allocproc failed\n");

            for (;;) 
            {
                asm volatile("wfi");
            }
        }

        uvminit(p);

        if (i == 0) 
        {
            safestrcpy(p->name, "init1", sizeof(p->name));
            initproc = p;
            first = p;
        } 
        else 
        {
            safestrcpy(p->name, "init2", sizeof(p->name));
        }

        p->state = RUNNABLE;

        printf("userinit: pid=%d name=%s state=RUNNABLE\n", p->pid, p->name);
    }
    

    return first;
}

struct proc *myproc(void)
{
    return current_proc;
}

void yield(void)
{
    struct proc *p = myproc();

    if (p == 0) 
    {
        printf("yield: no current proc\n");

        for (;;) 
        {
            asm volatile("wfi");
        }
    }

    printf("yield: pid=%d give up CPU\n", p->pid);

    /*
     * 当前进程主动让出 CPU。
     * 它不是结束了，而是重新变成 RUNNABLE，
     * 等待 scheduler 下次再选择它。
     */
    p->state = RUNNABLE;

    /*
     * 保存当前进程的内核上下文到 p->context，
     * 恢复 scheduler 的上下文。
     *
     * 以后 scheduler 再次选择这个进程时，
     * 会通过 swtch(&scheduler_context, &p->context)
     * 回到这里的 swtch 后面。
     */
    swtch(&p->context, &scheduler_context);

    /*
     * 当进程再次被 scheduler 选中后，
     * 会从这里继续执行。
     */
    printf("yield: pid=%d resumed\n", p->pid);
}

void proc_exit(int code)
{
    struct proc *p = myproc();

    if (p == 0) 
    {
        printf("proc_exit: no current proc\n");

        for (;;) 
        {
            asm volatile("wfi");
        }
    }

    printf("user exit, pid=%d code=%d\n", p->pid, code);

    p->state = ZOMBIE;

    swtch(&p->context, &scheduler_context);

    printf("proc_exit: ERROR: zombie process resumed, pid=%d\n", p->pid);

    for (;;) 
    {
        asm volatile("wfi");
    }
}

/*
 * forkret 是进程第一次被调度运行时进入的函数。
 *
 * 注意：
 * 这里名字叫 forkret，是借鉴 xv6 的命名。
 * 现在我们还没有 fork，所以它的意思只是：
 * “进程第一次从 scheduler 切换过来后，从这里继续执行”。
 */
static void forkret(void)
{
    struct proc *p = myproc();

    if (p == 0) 
    {
        printf("forkret: no current proc\n");

        for (;;) 
        {
            asm volatile("wfi");
        }
    }

    printf("forkret: pid=%d enter user mode...\n", p->pid);

    enter_user(p->entry, p->stack_top);

    printf("forkret: ERROR: enter_user returned.\n");

    for (;;) 
    {
        asm volatile("wfi");
    }
}

void scheduler(void)
{
    printf("scheduler start.\n");

    int next = 0;
    int idle_printed = 0;

    for (;;) 
    {
        int found = 0;
        
        for (int n = 0; n < NPROC; n++) 
        {
            int i = (next + n) % NPROC;
            struct proc *p = &proc[i];

            if (p->state == RUNNABLE) 
            {
                found = 1;
                idle_printed = 0;

                next = (i + 1) % NPROC;

                current_proc = p;
                p->state = RUNNING;

                /*
                 * 切换到当前进程自己的用户页表。
                 */
                uvminithart(p->pagetable);

                /*
                 * sscratch 保存当前进程的内核栈顶。
                 * 用户态 trap 进入内核时，trap.S 会用它切换到内核栈。
                 */
                w_sscratch(proc_kstack_top(p));

                printf("scheduler: swtch pid=%d name=%s\n", p->pid, p->name);

                /*
                 * 关键：
                 * 以前这里直接 enter_user。
                 * 现在改成 swtch 到进程的内核上下文。
                 *
                 * 第一次 swtch 到该进程时，会进入 forkret()。
                 */
                swtch(&scheduler_context, &p->context);

                printf("scheduler: back from pid=%d state=%d\n", p->pid, p->state);

                current_proc = 0;

                break;
            }
        }

        if (!found) 
        {
            if (!idle_printed)
            {
                printf("scheduler: no runnable process, idle.\n");
                idle_printed = 1;
            }
            
            asm volatile("wfi");
        }
    }
}