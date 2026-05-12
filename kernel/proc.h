#ifndef PROC_H
#define PROC_H

#include "riscv.h"
#include "vm.h"

#define NPROC 1

enum procstate 
{
    UNUSED,
    USED,
    RUNNABLE,
    RUNNING,
    ZOMBIE,
};

struct proc 
{
    int pid;
    enum procstate state;

    pagetable_t pagetable;

    uint64 entry;
    uint64 stack_top;

    char name[16];
};

extern struct proc *initproc;

void procinit(void);
struct proc *userinit(void);
struct proc *myproc(void);

#endif