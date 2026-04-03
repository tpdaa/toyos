#ifndef SBI_H
#define SBI_H

typedef unsigned long uint64;

int sbi_call(uint64 which,uint64 arg0,uint64 arg1,uint64 arg2);
void sbi_putchar(int ch);

#endif