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
#define SYS_getprogid 8
#define SYS_readfile 9
#define SYS_listfiles 10
#define SYS_createfile 11
#define SYS_statfile 12
#define SYS_unlinkfile 13
#define SYS_openfile 14
#define SYS_readfd 15
#define SYS_closefd 16
#define SYS_writefile 17
#define SYS_appendfile 18

#define PROG_SHELL 0
#define PROG_HELLO 1
#define PROG_COUNT 2
#define PROG_CAT 3
#define PROG_LS 4
#define PROG_CREATE 5
#define PROG_STAT 6
#define PROG_UNLINK 7
#define PROG_FDTEST 8
#define PROG_WRITE 9
#define PROG_APPEND 10

void syscall(struct trapframe *tf);

#endif