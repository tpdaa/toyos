#ifndef PROC_H
#define PROC_H

#include "riscv.h"
#include "vm.h"
#include "trapframe.h"

#define NPROC 8
#define KSTACK_SIZE 8192

enum procstate 
{
    UNUSED,
    USED,
    SLEEPING,
    RUNNABLE,
    RUNNING,
    ZOMBIE,
};

struct context
{
    uint64 ra;
    uint64 sp;

    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

struct proc 
{
    int pid;
    enum procstate state;

    pagetable_t pagetable;

    uint64 entry;
    uint64 stack_top;
    uint64 user_pc;
    uint64 sz;

    struct trapframe trapframe;
    struct context context;

    unsigned char kstack[KSTACK_SIZE] __attribute__((aligned(16)));

    struct proc *parent;
    int xstate;

    void *chan;
    char name[16];
};

extern struct proc *initproc;

void procinit(void);
struct proc *userinit(void);
struct proc *myproc(void);
uint64 proc_kstack_top(struct proc *p);
void scheduler(void);
void yield(void);
void proc_exit(int code);
int proc_fork(void);

int proc_wait(void);
void proc_sleep(void *chan);
void wakeup(void *chan);

#endif