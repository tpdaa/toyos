#ifndef USER_H
#define USER_H

#include "riscv.h"

#define USER_STACK_SIZE 16384

extern unsigned char user_stack[USER_STACK_SIZE];

void user_main(void);
void enter_user(uint64 entry, uint64 sp)__attribute__((noreturn));//entry：用户程序入口地址，sp：用户栈顶地址

#endif