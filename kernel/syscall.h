#ifndef SYSCALL_H
#define SYSCALL_H

#include "trapframe.h"

#define SYS_puts 1
#define SYS_exit 2
#define SYS_yield 3
#define SYS_getpid 4
#define SYS_fork 5
#define SYS_wait 6
#define SYS_exec 7


void syscall(struct trapframe *tf);

#endif