#ifndef UESR_H
#define USER_H

#include "riscv.h"

void user_main(void);
void enter_user(uint64 entry, uint64 sp);//entry：用户程序入口地址，sp：用户栈顶地址

#endif