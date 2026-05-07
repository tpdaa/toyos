#ifndef SYSCALL_H
#define SYSCALL_H

#include "trapframe.h"

#define SYS_puts 1
#define SYS_exit 2

void syscall(struct trapframe *tf);

#endif