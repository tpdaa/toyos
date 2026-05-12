#include "proc.h"
#include "printf.h"

static struct proc proc[NPROC];

struct proc *initproc;

static int nextpid = 1;

static void safestrcpy(char *dst, const char *src, int n)
{
    int i;

    if (n <= 0) {
        return;
    }

    for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }

    dst[i] = '\0';
}

void procinit(void)
{
    for (int i = 0; i < NPROC; i++) {
        proc[i].pid = 0;
        proc[i].state = UNUSED;
        proc[i].pagetable = 0;
        proc[i].entry = 0;
        proc[i].stack_top = 0;
        proc[i].name[0] = '\0';
    }

    initproc = 0;

    printf("proc init done.\n");
}

static struct proc *allocproc(void)
{
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].state == UNUSED) {
            struct proc *p = &proc[i];

            p->pid = nextpid++;
            p->state = USED;
            p->pagetable = 0;
            p->entry = 0;
            p->stack_top = 0;
            safestrcpy(p->name, "init", sizeof(p->name));

            return p;
        }
    }

    return 0;
}

struct proc *userinit(void)
{
    struct proc *p = allocproc();

    if (p == 0) {
        printf("userinit: allocproc failed\n");

        for (;;) {
            asm volatile("wfi");
        }
    }

    uvminit(p);

    p->state = RUNNING;
    initproc = p;

    printf("userinit done. pid=%d state=RUNNING\n", p->pid);

    return p;
}

struct proc *myproc(void)
{
    return initproc;
}