#include "proc.h"
#include "syscall.h"
#include "printf.h"

static struct proc proc[NPROC];

struct proc *initproc;

static struct proc *current_proc;

static struct context scheduler_context;

static int nextpid = 1;

extern void enter_user(uint64 entry, uint64 sp);
extern void usertrapret(struct trapframe *tf, uint64 user_pc, uint64 kstack_top);
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
        proc[i].user_pc = 0;
        proc[i].sz = 0;
        proc[i].parent = 0;
        proc[i].xstate = 0;
        proc[i].program_id = 0;
        proc[i].chan = 0;

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
            p->user_pc = 0;
            p->sz = 0;
            p->parent = 0;
            p->xstate = 0;
            p->program_id = 0;
            p->chan = 0;

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

    for (int i = 0; i < 1; i++) 
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
        p->program_id = 0;

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
        return;
    }

    //printf("yield: pid=%d give up CPU\n", p->pid);

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
    //printf("yield: pid=%d resumed\n", p->pid);
}

static void reparent(struct proc *p)
{
    if (initproc == 0)
    {
        return;
    }

    for (int i = 0; i < NPROC; i++)
    {
        struct proc *pp = &proc[i];

        if (pp->parent == p)
        {
            printf("reparent: child pid=%d from parent pid=%d to init pid=%d\n",
                   pp->pid, p->pid, initproc->pid);

            pp->parent = initproc;

            /*
             * 如果这个子进程已经是 ZOMBIE，
             * 那么 initproc 可能正在 wait，需要唤醒它。
             */
            if (pp->state == ZOMBIE)
            {
                wakeup(initproc);
            }
        }
    }
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


    /*
     * 当前进程退出前，如果它还有子进程，
     * 把这些子进程交给 initproc 接管。
     */
    reparent(p);

    p->xstate = code;
    p->state = ZOMBIE;

     /*
     * 子进程退出时唤醒父进程。
     * 如果父进程正在 wait 中睡眠，它会被改回 RUNNABLE。
     */
    if (p->parent != 0)
    {
        wakeup(p->parent);
    }

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

    printf("forkret: pid=%d enter user mode pc=%p\n",
           p->pid, (void *)p->user_pc);

    usertrapret(&p->trapframe, p->user_pc, proc_kstack_top(p));

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

                //printf("scheduler: swtch pid=%d name=%s\n", p->pid, p->name);

                /*
                 * 关键：
                 * 以前这里直接 enter_user。
                 * 现在改成 swtch 到进程的内核上下文。
                 *
                 * 第一次 swtch 到该进程时，会进入 forkret()。
                 */
                swtch(&scheduler_context, &p->context);

                //printf("scheduler: back from pid=%d state=%d\n", p->pid, p->state);

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

static void copy_trapframe(struct trapframe *dst, struct trapframe *src)
{
    uint64 *d = (uint64 *)dst;
    uint64 *s = (uint64 *)src;

    for (int i = 0; i < sizeof(struct trapframe) / sizeof(uint64); i++) 
    {
        d[i] = s[i];
    }
}


int proc_fork(void)
{
    struct proc *p = myproc();

    if (p == 0)
    {
        return -1;
    }

    struct proc *np = allocproc();

    if (np == 0)
    {
        printf("fork: allocproc failed\n");
        return -1;
    }

    np->pagetable = uvmcreate();

    if (np->pagetable == 0)
    {
        printf("fork: uvmcreate failed\n");
        np->state = UNUSED;
        return -1;
    }

    if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0)
    {
        printf("fork: uvmcopy failed\n");
        np->state = UNUSED;
        return -1;
    }

    /*
     * 复制父进程的用户态执行现场。
     */
    copy_trapframe(&np->trapframe, &p->trapframe);

    /*
     * fork 的关键语义：
     * 子进程从 fork 返回 0。
     */
    np->trapframe.a0 = 0;

    /*
     * 父子从同一个用户地址继续执行。
     */
    np->entry = p->entry;
    np->stack_top = p->stack_top;
    np->user_pc = p->user_pc;
    np->sz = p->sz;

    np->parent = p;
    np->xstate = 0;
    np->program_id = p->program_id;

    safestrcpy(np->name, "child", sizeof(np->name));

    np->state = RUNNABLE;

    printf("fork: parent pid=%d child pid=%d user_pc=%p\n",
           p->pid, np->pid, (void *)np->user_pc);

    /*
     * 父进程从 fork 返回子进程 pid。
     */
    return np->pid;
}

int proc_wait(void)
{
    struct proc *p = myproc();

    if (p == 0)
    {
        return -1;
    }

    for (;;)
    {
        int havekids = 0;

        for (int i = 0; i < NPROC; i++)
        {
            struct proc *np = &proc[i];

            if (np->parent != p)
            {
                continue;
            }

            havekids = 1;

            if (np->state == ZOMBIE)
            {
                int pid = np->pid;

                printf("wait: parent pid=%d collected child pid=%d code=%d\n",
                       p->pid, np->pid, np->xstate);

                if (np->pagetable != 0)
                {
                    uvmfree(np->pagetable, np->sz);
                }

                np->pid = 0;
                np->state = UNUSED;
                np->pagetable = 0;
                np->entry = 0;
                np->stack_top = 0;
                np->user_pc = 0;
                np->sz = 0;
                np->parent = 0;
                np->xstate = 0;
                np->name[0] = '\0';
                np->program_id = 0;
                np->chan = 0;

                return pid;
            }
        }

        if (!havekids)
        {
            return -1;
        }

        proc_sleep(p);

    }
}

void proc_sleep(void *chan)
{
    struct proc *p = myproc();

    if (p == 0)
    {
        printf("proc_sleep: no current proc\n");
        return;
    }

    /*
     * 当前进程睡在 chan 这个等待通道上。
     * scheduler 不会再调度 SLEEPING 进程。
     */
    p->chan = chan;
    p->state = SLEEPING;

    printf("sleep: pid=%d chan=%p\n", p->pid, chan);

    /*
     * 切回 scheduler。
     * 以后被 wakeup 改回 RUNNABLE 后，
     * scheduler 再次调度它时，会从这里继续返回。
     */
    swtch(&p->context, &scheduler_context);

    /*
     * 被唤醒并重新运行后，清空 chan。
     */
    p->chan = 0;
}

void wakeup(void *chan)
{
    for (int i = 0; i < NPROC; i++)
    {
        struct proc *p = &proc[i];

        if (p->state == SLEEPING && p->chan == chan)
        {
            printf("wakeup: pid=%d chan=%p\n", p->pid, chan);

            p->state = RUNNABLE;
        }
    }
}

int proc_exec(int program_id)
{
    struct proc *p = myproc();

    if (p == 0)
    {
        return -1;
    }

    if (program_id < PROG_SHELL || program_id > PROG_CAT)
    {
        printf("exec: bad program_id=%d\n", program_id);
        return -1;
    }

    printf("exec: pid=%d reload user image program=%d\n",
           p->pid, program_id);

    /*
     * 释放当前进程旧的用户地址空间。
     */
    if (p->pagetable != 0)
    {
        uvmfree(p->pagetable, p->sz);
        p->pagetable = 0;
    }

    p->program_id = program_id;

    /*
     * 重新加载内置用户程序。
     * uvminit 会重新创建 pagetable，
     * 重新映射用户代码、数据、栈，
     * 并设置 entry、stack_top、user_pc、sz。
     */
    uvminit(p);

     /*
     * exec 后当前 CPU 必须切到新的用户页表。
     * 因为旧页表已经被 uvmfree 释放了。
     */
    uvminithart(p->pagetable);
    /*
     * exec 成功后，当前进程应该从新程序入口开始运行。
     */
    p->user_pc = p->entry;
    p->trapframe.sp = p->stack_top;

    /*
     * exec 系统调用本身不再返回到旧用户代码。
     * 返回 0 只是让 syscall 层有一个正常返回值；
     * 真正返回用户态的位置由 p->user_pc 决定。
     */
    return 0;
}

int proc_get_program_id(void)
{
    struct proc *p = myproc();

    if (p == 0)
    {
        return -1;
    }

    return p->program_id;
}